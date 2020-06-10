/*
 * Copyright (c) 1999-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sched.h>
#include "scno.h"

#ifndef CSIGNAL
# define CSIGNAL 0x000000ff
#endif

#include "print_fields.h"

#include "xlat/clone_flags.h"
#include "xlat/clone3_flags.h"
#include "xlat/setns_types.h"
#include "xlat/unshare_flags.h"

#if defined IA64
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_STACKSIZE	(shuffle_scno(tcp->scno) == __NR_clone2 ? 2 : -1)
# define ARG_PTID	(shuffle_scno(tcp->scno) == __NR_clone2 ? 3 : 2)
# define ARG_CTID	(shuffle_scno(tcp->scno) == __NR_clone2 ? 4 : 3)
# define ARG_TLS	(shuffle_scno(tcp->scno) == __NR_clone2 ? 5 : 4)
#elif defined S390 || defined S390X
# define ARG_STACK	0
# define ARG_FLAGS	1
# define ARG_PTID	2
# define ARG_CTID	3
# define ARG_TLS	4
#elif defined X86_64 || defined X32
/* x86 personality processes have the last two arguments flipped. */
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_CTID	((current_personality != 1) ? 3 : 4)
# define ARG_TLS	((current_personality != 1) ? 4 : 3)
#elif defined ALPHA || defined TILE || defined OR1K || defined CSKY
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_CTID	3
# define ARG_TLS	4
#else
# define ARG_FLAGS	0
# define ARG_STACK	1
# define ARG_PTID	2
# define ARG_TLS	3
# define ARG_CTID	4
#endif

static void
print_tls_arg(struct tcb *const tcp, const kernel_ulong_t addr)
{
#ifdef HAVE_STRUCT_USER_DESC
	if ((SUPPORTED_PERSONALITIES == 1) || (current_personality == 1))
	{
		print_user_desc(tcp, addr, USER_DESC_BOTH);
	}
	else
#endif /* HAVE_STRUCT_USER_DESC */
	{
		printaddr(addr);
	}
}

SYS_FUNC(clone)
{
	const kernel_ulong_t flags = tcp->u_arg[ARG_FLAGS] & ~CSIGNAL;

	if (entering(tcp)) {
		const unsigned int sig = tcp->u_arg[ARG_FLAGS] & CSIGNAL;

		tprints("child_stack=");
		printaddr(tcp->u_arg[ARG_STACK]);
		tprints(", ");
#ifdef ARG_STACKSIZE
		if (ARG_STACKSIZE != -1)
			tprintf("stack_size=%#" PRI_klx ", ",
				tcp->u_arg[ARG_STACKSIZE]);
#endif
		tprints("flags=");
		if (flags) {
			printflags64(clone_flags, flags, "CLONE_???");
			if (sig) {
				tprints("|");
				printsignal(sig);
			}
		} else {
			printsignal(sig);
		}
		/*
		 * TODO on syscall entry:
		 * We can clear CLONE_PTRACE here since it is an ancient hack
		 * to allow us to catch children, and we use another hack for that.
		 * But CLONE_PTRACE can conceivably be used by malicious programs
		 * to subvert us. By clearing this bit, we can defend against it:
		 * in untraced execution, CLONE_PTRACE should have no effect.
		 *
		 * We can also clear CLONE_UNTRACED, since it allows to start
		 * children outside of our control. At the moment
		 * I'm trying to figure out whether there is a *legitimate*
		 * use of this flag which we should respect.
		 */
		if ((flags & (CLONE_PARENT_SETTID|CLONE_PIDFD|CLONE_CHILD_SETTID
			      |CLONE_CHILD_CLEARTID|CLONE_SETTLS)) == 0)
			return RVAL_DECODED | RVAL_TID;
	} else {
		if (flags & (CLONE_PARENT_SETTID|CLONE_PIDFD)) {
			kernel_ulong_t addr = tcp->u_arg[ARG_PTID];

			tprints(", parent_tid=");
			if (flags & CLONE_PARENT_SETTID)
				printnum_pid(tcp, addr, PT_TID);
			else
				printnum_fd(tcp, addr);
		}
		if (flags & CLONE_SETTLS) {
			tprints(", tls=");
			print_tls_arg(tcp, tcp->u_arg[ARG_TLS]);
		}
		if (flags & (CLONE_CHILD_SETTID|CLONE_CHILD_CLEARTID)) {
			tprints(", child_tidptr=");
			printaddr(tcp->u_arg[ARG_CTID]);
		}
	}
	return RVAL_TID;
}


struct strace_clone_args {
	uint64_t flags;
	uint64_t /* fd * */    pidfd;
	uint64_t /* pid_t * */ child_tid;
	uint64_t /* pid_t * */ parent_tid;
	uint64_t /* int */     exit_signal;
	uint64_t /* void * */  stack;
	uint64_t stack_size;
	uint64_t /* struct user_desc * / void * */ tls;
	uint64_t /* pid_t * */ set_tid;
	uint64_t set_tid_size;
	uint64_t cgroup;
};

SYS_FUNC(clone3)
{
	static const size_t minsz = offsetofend(struct strace_clone_args, tls);

	const kernel_ulong_t addr = tcp->u_arg[0];
	const kernel_ulong_t size = tcp->u_arg[1];

	struct strace_clone_args arg = { 0 };
	kernel_ulong_t fetch_size;

	fetch_size = MIN(size, sizeof(arg));

	if (entering(tcp)) {
		if (fetch_size < minsz) {
			printaddr(addr);
			goto out;
		} else if (umoven_or_printaddr(tcp, addr, fetch_size, &arg)) {
			goto out;
		}

		tprints("{flags=");
		printflags_ex(arg.flags, "CLONE_???", XLAT_STYLE_DEFAULT,
			      clone_flags, clone3_flags, NULL);

		if (arg.flags & CLONE_PIDFD)
			PRINT_FIELD_ADDR64(", ", arg, pidfd);

		if (arg.flags & (CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID))
			PRINT_FIELD_ADDR64(", ", arg, child_tid);

		if (arg.flags & CLONE_PARENT_SETTID)
			PRINT_FIELD_ADDR64(", ", arg, parent_tid);

		tprints(", exit_signal=");
		if (arg.exit_signal < INT_MAX)
			printsignal(arg.exit_signal);
		else
			tprintf("%" PRIu64, arg.exit_signal);

		PRINT_FIELD_ADDR64(", ", arg, stack);
		PRINT_FIELD_X(", ", arg, stack_size);

		if (arg.flags & CLONE_SETTLS) {
			tprints(", tls=");
			print_tls_arg(tcp, arg.tls);
		}

		if (arg.set_tid || arg.set_tid_size) {
			static const unsigned int max_set_tid_size = 32;

			if (!arg.set_tid || !arg.set_tid_size ||
			    arg.set_tid_size > max_set_tid_size) {
				PRINT_FIELD_ADDR64(", ", arg, set_tid);
			} else {
				int buf;

				tprints(", set_tid=");
				print_array(tcp, arg.set_tid, arg.set_tid_size,
					    &buf, sizeof(buf), tfetch_mem,
					    print_int32_array_member, 0);
			}
			PRINT_FIELD_U(", ", arg, set_tid_size);
		}

		if (fetch_size > offsetof(struct strace_clone_args, cgroup)
		    && (arg.cgroup || arg.flags & CLONE_INTO_CGROUP))
			PRINT_FIELD_U(", ", arg, cgroup);

		if (size > fetch_size)
			print_nonzero_bytes(tcp, ", ", addr, fetch_size,
					    MIN(size, get_pagesize()),
					    QUOTE_FORCE_HEX);

		tprints("}");

		if ((arg.flags & (CLONE_PIDFD | CLONE_PARENT_SETTID)) ||
		    (size > fetch_size))
			return RVAL_TID;

		goto out;
	}

	/* exiting */

	if (syserror(tcp))
		goto out;

	if (umoven(tcp, addr, fetch_size, &arg)) {
		tprints(" => ");
		printaddr(addr);
		goto out;
	}

	static const char initial_pfx[] = " => {";
	const char *pfx = initial_pfx;

	if (arg.flags & CLONE_PIDFD) {
		tprintf("%spidfd=", pfx);
		printnum_fd(tcp, arg.pidfd);
		pfx = ", ";
	}

	if (arg.flags & CLONE_PARENT_SETTID) {
		tprintf("%sparent_tid=", pfx);
		printnum_pid(tcp, arg.parent_tid, PT_TID);
		pfx = ", ";
	}

	if (size > fetch_size) {
		/*
		 * TODO: it is possible to also store the tail on entering
		 *       and then compare against it on exiting in order
		 *       to avoid double-printing, but it would also require yet
		 *       another complication of print_nonzero_bytes interface.
		 */
		if (print_nonzero_bytes(tcp, pfx, addr, fetch_size,
					MIN(size, get_pagesize()),
					QUOTE_FORCE_HEX))
			pfx = ", ";
	}

	if (pfx != initial_pfx)
		tprints("}");

out:
	tprintf(", %" PRI_klu, size);

	return RVAL_DECODED | RVAL_TID;
}


SYS_FUNC(setns)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printxval(setns_types, tcp->u_arg[1], "CLONE_NEW???");

	return RVAL_DECODED;
}

SYS_FUNC(unshare)
{
	printflags64(unshare_flags, tcp->u_arg[0], "CLONE_???");
	return RVAL_DECODED;
}

SYS_FUNC(fork)
{
	return RVAL_DECODED | RVAL_TGID;
}
