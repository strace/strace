#include "defs.h"

#include <sys/wait.h>

#ifndef __WNOTHREAD
# define __WNOTHREAD	0x20000000
#endif
#ifndef __WALL
# define __WALL		0x40000000
#endif
#ifndef __WCLONE
# define __WCLONE	0x80000000
#endif

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
printwaitn(struct tcb *tcp, int n, int bitness)
{
	int status;

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
		/* status */
		if (!tcp->u_arg[1])
			tprints("NULL");
		else if (syserror(tcp) || tcp->u_rval == 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &status) < 0)
			tprints("[?]");
		else
			printstatus(status);
		/* options */
		tprints(", ");
		printflags(wait4_options, tcp->u_arg[2], "W???");
		if (n == 4) {
			tprints(", ");
			/* usage */
			if (!tcp->u_arg[3])
				tprints("NULL");
			else if (tcp->u_rval > 0) {
#ifdef ALPHA
				if (bitness)
					printrusage32(tcp, tcp->u_arg[3]);
				else
#endif
					printrusage(tcp, tcp->u_arg[3]);
			}
			else
				tprintf("%#lx", tcp->u_arg[3]);
		}
	}
	return 0;
}

SYS_FUNC(waitpid)
{
	return printwaitn(tcp, 3, 0);
}

SYS_FUNC(wait4)
{
	return printwaitn(tcp, 4, 0);
}

#ifdef ALPHA
SYS_FUNC(osf_wait4)
{
	return printwaitn(tcp, 4, 1);
}
#endif

#include "xlat/waitid_types.h"

SYS_FUNC(waitid)
{
	if (entering(tcp)) {
		printxval(waitid_types, tcp->u_arg[0], "P_???");
		tprintf(", %ld, ", tcp->u_arg[1]);
	}
	else {
		/* siginfo */
		printsiginfo_at(tcp, tcp->u_arg[2]);
		/* options */
		tprints(", ");
		printflags(wait4_options, tcp->u_arg[3], "W???");
		if (tcp->s_ent->nargs > 4) {
			/* usage */
			tprints(", ");
			if (!tcp->u_arg[4])
				tprints("NULL");
			else if (tcp->u_error)
				tprintf("%#lx", tcp->u_arg[4]);
			else
				printrusage(tcp, tcp->u_arg[4]);
		}
	}
	return 0;
}
