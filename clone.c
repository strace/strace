/*
 * Copyright (c) 1999-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sched.h>
#include <asm/unistd.h>

#ifndef CSIGNAL
# define CSIGNAL 0x000000ff
#endif

#include "xlat/clone_flags.h"
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
# if SUPPORTED_PERSONALITIES > 1
	if (current_personality == 1)
# endif
	{
		print_user_desc(tcp, addr, USER_DESC_BOTH);
	}
# if SUPPORTED_PERSONALITIES > 1
	else
# endif
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
			if (sig)
				printsignal(sig);
			else
				tprints("0");
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
			return RVAL_DECODED;
	} else {
		if (flags & (CLONE_PARENT_SETTID|CLONE_PIDFD)) {
			kernel_ulong_t addr = tcp->u_arg[ARG_PTID];

			tprints(", parent_tid=");
			if (flags & CLONE_PARENT_SETTID)
				printnum_int(tcp, addr, "%u");
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
	return 0;
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
	return RVAL_DECODED;
}
