/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#include <sys/wait.h>

#include "xlat/wait4_options.h"

#if !defined WCOREFLAG && defined WCOREFLG
# define WCOREFLAG WCOREFLG
#endif
#ifndef WCOREFLAG
# define WCOREFLAG 0x80
#endif
#ifndef WCOREDUMP
# define WCOREDUMP(status)  ((status) & 0200)
#endif
#ifndef W_STOPCODE
# define W_STOPCODE(sig)  ((sig) << 8 | 0x7f)
#endif
#ifndef W_EXITCODE
# define W_EXITCODE(ret, sig)  ((ret) << 8 | (sig))
#endif
#ifndef W_CONTINUED
# define W_CONTINUED 0xffff
#endif

#include "ptrace.h"
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
	if (WIFSTOPPED(status)) {
		int sig = WSTOPSIG(status);
		tprintf("[{WIFSTOPPED(s) && WSTOPSIG(s) == %s%s}",
			signame(sig & 0x7f),
			sig & 0x80 ? " | 0x80" : "");
		status &= ~W_STOPCODE(sig);
	}
	else if (WIFSIGNALED(status)) {
		tprintf("[{WIFSIGNALED(s) && WTERMSIG(s) == %s%s}",
			signame(WTERMSIG(status)),
			WCOREDUMP(status) ? " && WCOREDUMP(s)" : "");
		status &= ~(W_EXITCODE(0, WTERMSIG(status)) | WCOREFLAG);
	}
	else if (WIFEXITED(status)) {
		tprintf("[{WIFEXITED(s) && WEXITSTATUS(s) == %d}",
			WEXITSTATUS(status));
		exited = 1;
		status &= ~W_EXITCODE(WEXITSTATUS(status), 0);
	}
#ifdef WIFCONTINUED
	else if (WIFCONTINUED(status)) {
		tprints("[{WIFCONTINUED(s)}");
		status &= ~W_CONTINUED;
	}
#endif
	else {
		tprintf("[%#x]", status);
		return 0;
	}

	if (status) {
		unsigned int event = (unsigned int) status >> 16;
		if (event) {
			tprints(" | ");
			printxval(ptrace_events, event, "PTRACE_EVENT_???");
			tprints(" << 16");
			status &= 0xffff;
		}
		if (status)
			tprintf(" | %#x", status);
	}
	tprints("]");

	return exited;
}

static int
printwaitn(struct tcb *const tcp,
	   void (*const print_rusage)(struct tcb *, kernel_ulong_t))
{
	if (entering(tcp)) {
		/* On Linux, kernel-side pid_t is typedef'ed to int
		 * on all arches. Also, glibc-2.8 truncates wait3 and wait4
		 * pid argument to int on 64bit arches, producing,
		 * for example, wait4(4294967295, ...) instead of -1
		 * in strace. We have to use int here, not long.
		 */
		int pid = tcp->u_arg[0];
		tprintf("%d, ", pid);
	} else {
		int status;

		/* status */
		if (tcp->u_rval == 0)
			printaddr(tcp->u_arg[1]);
		else if (!umove_or_printaddr(tcp, tcp->u_arg[1], &status))
			printstatus(status);
		/* options */
		tprints(", ");
		printflags(wait4_options, tcp->u_arg[2], "W???");
		if (print_rusage) {
			/* usage */
			tprints(", ");
			if (tcp->u_rval > 0)
				print_rusage(tcp, tcp->u_arg[3]);
			else
				printaddr(tcp->u_arg[3]);
		}
	}
	return 0;
}

SYS_FUNC(waitpid)
{
	return printwaitn(tcp, NULL);
}

SYS_FUNC(wait4)
{
	return printwaitn(tcp, printrusage);
}

#ifdef ALPHA
SYS_FUNC(osf_wait4)
{
	return printwaitn(tcp, printrusage32);
}
#endif

#include "xlat/waitid_types.h"

SYS_FUNC(waitid)
{
	if (entering(tcp)) {
		printxval(waitid_types, tcp->u_arg[0], "P_???");
		int pid = tcp->u_arg[1];
		tprintf(", %d, ", pid);
	} else {
		/* siginfo */
		printsiginfo_at(tcp, tcp->u_arg[2]);
		/* options */
		tprints(", ");
		printflags(wait4_options, tcp->u_arg[3], "W???");
		/* usage */
		tprints(", ");
		printrusage(tcp, tcp->u_arg[4]);
	}
	return 0;
}
