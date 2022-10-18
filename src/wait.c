/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ptrace.h"

#include "wait.h"

#include "xlat/wait4_options.h"
#include "xlat/ptrace_events.h"

static int
printstatus(int status)
{
	int exited = 0;

	/*
	 * Here is a tricky presentation problem.  This solution
	 * is still not entirely satisfactory but since there
	 * are no wait status constructors it will have to do.
	 */
	tprint_indirect_begin();
	tprint_flags_begin();
	if (WIFSTOPPED(status)) {
		int sig = WSTOPSIG(status);
		tprintf_string("{WIFSTOPPED(s) && WSTOPSIG(s) == %s%s}",
			       sprintsigname(sig & 0x7f),
			       sig & 0x80 ? " | 0x80" : "");
		status &= ~W_STOPCODE(sig);
	} else if (WIFSIGNALED(status)) {
		tprintf_string("{WIFSIGNALED(s) && WTERMSIG(s) == %s%s}",
			       sprintsigname(WTERMSIG(status)),
			       WCOREDUMP(status) ? " && WCOREDUMP(s)" : "");
		status &= ~(W_EXITCODE(0, WTERMSIG(status)) | WCOREFLAG);
	} else if (WIFEXITED(status)) {
		tprintf_string("{WIFEXITED(s) && WEXITSTATUS(s) == %d}",
			       WEXITSTATUS(status));
		exited = 1;
		status &= ~W_EXITCODE(WEXITSTATUS(status), 0);
	}
#ifdef WIFCONTINUED
	else if (WIFCONTINUED(status)) {
		tprints_string("{WIFCONTINUED(s)}");
		status &= ~W_CONTINUED;
	}
#endif
	else {
		PRINT_VAL_X(status);
		tprint_flags_end();
		tprint_indirect_end();
		return 0;
	}

	if (status) {
		unsigned int event = (unsigned int) status >> 16;
		if (event) {
			tprint_flags_or();
			tprint_shift_begin();
			printxval(ptrace_events, event, "PTRACE_EVENT_???");
			tprint_shift();
			PRINT_VAL_U(16);
			tprint_shift_end();
			status &= 0xffff;
		}
		if (status) {
			tprint_flags_or();
			PRINT_VAL_X(status);
		}
	}
	tprint_flags_end();
	tprint_indirect_end();

	return exited;
}

static int
printwaitn(struct tcb *const tcp,
	   void (*const print_rusage)(struct tcb *, kernel_ulong_t))
{
	if (entering(tcp)) {
		/* pid */
		printpid_tgid_pgid(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		int status;

		/* status */
		if (tcp->u_rval == 0)
			printaddr(tcp->u_arg[1]);
		else if (!umove_or_printaddr(tcp, tcp->u_arg[1], &status))
			printstatus(status);
		tprint_arg_next();

		/* options */
		printflags(wait4_options, tcp->u_arg[2], "W???");
		if (print_rusage) {
			tprint_arg_next();

			/* usage */
			if (tcp->u_rval > 0)
				print_rusage(tcp, tcp->u_arg[3]);
			else
				printaddr(tcp->u_arg[3]);
		}
	}
	return RVAL_TGID;
}

SYS_FUNC(waitpid)
{
	return printwaitn(tcp, NULL);
}

#if HAVE_ARCH_TIME32_SYSCALLS || HAVE_ARCH_OLD_TIME64_SYSCALLS
SYS_FUNC(wait4)
{
	return printwaitn(tcp, printrusage);
}
#endif

#ifdef ALPHA
SYS_FUNC(osf_wait4)
{
	return printwaitn(tcp, printrusage32);
}
#endif

#include "xlat/waitid_types.h"

SYS_FUNC(waitid)
{
	unsigned int idtype = (unsigned int) tcp->u_arg[0];
	int id = tcp->u_arg[1];

	if (entering(tcp)) {
		/* idtype */
		printxval(waitid_types, idtype, "P_???");
		tprint_arg_next();

		switch (idtype) {
		case P_PID:
			printpid(tcp, id, PT_TGID);
			break;
		case P_PIDFD:
			printfd(tcp, id);
			break;
		case P_PGID:
			printpid(tcp, id, PT_PGID);
			break;
		default:
			PRINT_VAL_D(id);
			break;
		}
		tprint_arg_next();
	} else {
		/* siginfo */
		printsiginfo_at(tcp, tcp->u_arg[2]);
		tprint_arg_next();

		/* options */
		printflags(wait4_options, tcp->u_arg[3], "W???");
		tprint_arg_next();

		/* usage */
		printrusage(tcp, tcp->u_arg[4]);
	}
	return 0;
}
