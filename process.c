/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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
 *
 *	$Id$
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <signal.h>
#ifdef SUNOS4
#include <machine/reg.h>
#endif /* SUNOS4 */

#if __GLIBC__ == 2 && __GLIBC_MINOR__ >= 1 && (defined(I386) || defined(M68K))
# include <sys/reg.h>
# define PTRACE_PEEKUSR PTRACE_PEEKUSER
# define PTRACE_POKEUSR PTRACE_POKEUSER
#endif

#ifdef LINUX
#ifndef __GLIBC__
#include <linux/ptrace.h>
#endif
#include <asm/posix_types.h>
#undef GETGROUPS_T
#define GETGROUPS_T __kernel_gid_t
#endif /* LINUX */

#ifdef HAVE_PRCTL
#include <sys/prctl.h>
#endif

#ifndef WCOREDUMP
#define WCOREDUMP(status) ((status) & 0200)
#endif

/* WTA: this has `&& !defined(LINUXSPARC)', this seems unneeded though? */
#if defined(HAVE_PRCTL)
static struct xlat prctl_options[] = {
#ifdef PR_MAXPROCS
	{ PR_MAXPROCS,		"PR_MAXPROCS"		},
#endif
#ifdef PR_ISBLOCKED
	{ PR_ISBLOCKED,		"PR_ISBLOCKED"		},
#endif
#ifdef PR_SETSTACKSIZE
	{ PR_SETSTACKSIZE,	"PR_SETSTACKSIZE"	},
#endif
#ifdef PR_GETSTACKSIZE
	{ PR_GETSTACKSIZE,	"PR_GETSTACKSIZE"	},
#endif
#ifdef PR_MAXPPROCS
	{ PR_MAXPPROCS,		"PR_MAXPPROCS"		},
#endif
#ifdef PR_UNBLKONEXEC
	{ PR_UNBLKONEXEC,	"PR_UNBLKONEXEC"	},
#endif
#ifdef PR_ATOMICSIM
	{ PR_ATOMICSIM,		"PR_ATOMICSIM"		},
#endif
#ifdef PR_SETEXITSIG
	{ PR_SETEXITSIG,	"PR_SETEXITSIG"		},
#endif
#ifdef PR_RESIDENT
	{ PR_RESIDENT,		"PR_RESIDENT"		},
#endif
#ifdef PR_ATTACHADDR
	{ PR_ATTACHADDR,	"PR_ATTACHADDR"		},
#endif
#ifdef PR_DETACHADDR
	{ PR_DETACHADDR,	"PR_DETACHADDR"		},
#endif
#ifdef PR_TERMCHILD
	{ PR_TERMCHILD,		"PR_TERMCHILD"		},
#endif
#ifdef PR_GETSHMASK
	{ PR_GETSHMASK,		"PR_GETSHMASK"		},
#endif
#ifdef PR_GETNSHARE
	{ PR_GETNSHARE,		"PR_GETNSHARE"		},
#endif
#if defined(PR_SET_PDEATHSIG)
	{ PR_SET_PDEATHSIG,	"PR_SET_PDEATHSIG"	},
#endif
#ifdef PR_COREPID
	{ PR_COREPID,		"PR_COREPID"		},
#endif
#ifdef PR_ATTACHADDRPERM
	{ PR_ATTACHADDRPERM,	"PR_ATTACHADDRPERM"	},
#endif
#ifdef PR_PTHREADEXIT
	{ PR_PTHREADEXIT,	"PR_PTHREADEXIT"	},
#endif
	{ 0,			NULL			},
};

int
sys_prctl(tcp)
struct tcb *tcp;
{
	int i;

	if (entering(tcp)) {
		printxval(prctl_options, tcp->u_arg[0], "PR_???");
		switch (tcp->u_arg[0]) {
#ifdef PR_GETNSHARE
		case PR_GETNSHARE:
			break;
#endif
		default:
			for (i = 1; i < tcp->u_nargs; i++)
				tprintf(", %#lx", tcp->u_arg[i]);
			break;
		}
	}
	return 0;
}

#endif /* HAVE_PRCTL */

int
sys_gethostid(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_HEX;
	return 0;
}

int
sys_sethostname(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpathn(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_gethostname(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setdomainname(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpathn(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

#if !defined(LINUX)

int
sys_getdomainname(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}
#endif /* !LINUX */

int
sys_exit(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		fprintf(stderr, "_exit returned!\n");
		return -1;
	}
	/* special case: we stop tracing this process, finish line now */
	tprintf("%ld) ", tcp->u_arg[0]);
	tabto(acolumn);
	tprintf("= ?");
	printtrailer(tcp);
	return 0;
}

int
internal_exit(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tcp->flags |= TCB_EXITING;
	return 0;
}

#ifdef SVR4

int
sys_fork(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		if (getrval2(tcp)) {
			tcp->auxstr = "child process";
			return RVAL_UDECIMAL | RVAL_STR;
		}
	}
	return 0;
}

int
internal_fork(tcp)
struct tcb *tcp;
{
	struct tcb *tcpchild;

	if (exiting(tcp)) {
		if (getrval2(tcp))
			return 0;
		if (!followfork)
			return 0;
		if (nprocs == MAX_PROCS) {
			tcp->flags &= ~TCB_FOLLOWFORK;
			fprintf(stderr, "sys_fork: tcb table full\n");
			return 0;
		}
		else
			tcp->flags |= TCB_FOLLOWFORK;
		if (syserror(tcp))
			return 0;
		if ((tcpchild = alloctcb(tcp->u_rval)) == NULL) {
			fprintf(stderr, "sys_fork: tcb table full\n");
			return 0;
		}
		proc_open(tcpchild, 1);
	}
	return 0;
}

#else /* !SVR4 */

int
sys_fork(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}

int
internal_fork(tcp)
struct tcb *tcp;
{
	struct tcb *tcpchild;
	int pid;
	int dont_follow = 0;

#ifdef SYS_vfork
	if (tcp->scno == SYS_vfork) {
#if defined(I386) && defined(LINUX)
		/* Attempt to make vfork into fork, which we can follow. */
		if (!followvfork || 
		    ptrace(PTRACE_POKEUSR, tcp->pid, 
			   (void *)(ORIG_EAX * 4), SYS_fork) < 0) 
			dont_follow = 1;
		
#else
		dont_follow = 1;
#endif
	}
#endif
#ifdef SYS_clone
	/* clone can do many things, not all of which we know how to handle.
	   Don't do it for now. */
	if (tcp->scno == SYS_clone)
		dont_follow = 1;
#endif
	if (entering(tcp)) {
		if (!followfork || dont_follow)
			return 0;
		if (nprocs == MAX_PROCS) {
			tcp->flags &= ~TCB_FOLLOWFORK;
			fprintf(stderr, "sys_fork: tcb table full\n");
			return 0;
		}
		tcp->flags |= TCB_FOLLOWFORK;
		if (setbpt(tcp) < 0)
			return 0;
	}
	else {
		int bpt = tcp->flags & TCB_BPTSET;

		if (!(tcp->flags & TCB_FOLLOWFORK))
			return 0;
		if (bpt)
			clearbpt(tcp);

		if (syserror(tcp))
			return 0;

		pid = tcp->u_rval;
		if ((tcpchild = alloctcb(pid)) == NULL) {
			fprintf(stderr, " [tcb table full]\n");
			kill(pid, SIGKILL); /* XXX */
			return 0;
		}
#ifdef LINUX
		if (ptrace(PTRACE_ATTACH, pid, (char *) 1, 0) < 0) {
			perror("PTRACE_ATTACH");
			fprintf(stderr, "Too late?\n");
			droptcb(tcpchild);
			return 0;
		}
#endif /* LINUX */
#ifdef SUNOS4
#ifdef oldway
		/* The child must have run before it can be attached. */
		{
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
			select(0, NULL, NULL, NULL, &tv);
		}
		if (ptrace(PTRACE_ATTACH, pid, (char *)1, 0) < 0) {
			perror("PTRACE_ATTACH");
			fprintf(stderr, "Too late?\n");
			droptcb(tcpchild);
			return 0;
		}
#else /* !oldway */
		/* Try to catch the new process as soon as possible. */
		{
			int i;
			for (i = 0; i < 1024; i++)
				if (ptrace(PTRACE_ATTACH, pid, (char *) 1, 0) >= 0)
					break;
			if (i == 1024) {
				perror("PTRACE_ATTACH");
				fprintf(stderr, "Too late?\n");
				droptcb(tcpchild);
				return 0;
			}
		}
#endif /* !oldway */
#endif /* SUNOS4 */
		tcpchild->flags |= TCB_ATTACHED;
		/* Child has BPT too, must be removed on first occasion */
		if (bpt) {
			tcpchild->flags |= TCB_BPTSET;
			tcpchild->baddr = tcp->baddr;
			memcpy(tcpchild->inst, tcp->inst,
				sizeof tcpchild->inst);
		}
		newoutf(tcpchild);
		tcpchild->parent = tcp;
		tcp->nchildren++;
		if (!qflag)
			fprintf(stderr, "Process %d attached\n", pid);
	}
	return 0;
}

#endif /* !SVR4 */

#if defined(SUNOS4) || defined(LINUX)

int
sys_vfork(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}

#endif /* SUNOS4 || LINUX */

#ifndef LINUX

static char idstr[16];

int
sys_getpid(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		sprintf(idstr, "ppid %lu", getrval2(tcp));
		tcp->auxstr = idstr;
		return RVAL_STR;
	}
	return 0;
}

int
sys_getuid(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		sprintf(idstr, "euid %lu", getrval2(tcp));
		tcp->auxstr = idstr;
		return RVAL_STR;
	}
	return 0;
}

int
sys_getgid(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		sprintf(idstr, "egid %lu", getrval2(tcp));
		tcp->auxstr = idstr;
		return RVAL_STR;
	}
	return 0;
}

#endif /* !LINUX */

#ifdef LINUX

int
sys_setuid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%u", (uid_t) tcp->u_arg[0]);
	}
	return 0;
}

int
sys_setgid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%u", (gid_t) tcp->u_arg[0]);
	}
	return 0;
}

int
sys_getresuid(tcp)
    struct tcb *tcp;
{
	if (exiting(tcp)) {
		__kernel_uid_t uid;
		if (syserror(tcp))
			tprintf("%#lx, %#lx, %#lx", tcp->u_arg[0],
				tcp->u_arg[1], tcp->u_arg[2]);
		else {
			if (umove(tcp, tcp->u_arg[0], &uid) < 0)
				tprintf("%#lx, ", tcp->u_arg[0]);
			else
				tprintf("ruid %lu, ", (unsigned long) uid);
			if (umove(tcp, tcp->u_arg[0], &uid) < 0)
				tprintf("%#lx, ", tcp->u_arg[0]);
			else
				tprintf("euid %lu, ", (unsigned long) uid);
			if (umove(tcp, tcp->u_arg[0], &uid) < 0)
				tprintf("%#lx", tcp->u_arg[0]);
			else
				tprintf("suid %lu", (unsigned long) uid);
		}
	}
	return 0;
}

int
sys_getresgid(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		__kernel_gid_t gid;
		if (syserror(tcp))
			tprintf("%#lx, %#lx, %#lx", tcp->u_arg[0],
				tcp->u_arg[1], tcp->u_arg[2]);
		else {
			if (umove(tcp, tcp->u_arg[0], &gid) < 0)
				tprintf("%#lx, ", tcp->u_arg[0]);
			else
				tprintf("rgid %lu, ", (unsigned long) gid);
			if (umove(tcp, tcp->u_arg[0], &gid) < 0)
				tprintf("%#lx, ", tcp->u_arg[0]);
			else
				tprintf("egid %lu, ", (unsigned long) gid);
			if (umove(tcp, tcp->u_arg[0], &gid) < 0)
				tprintf("%#lx", tcp->u_arg[0]);
			else
				tprintf("sgid %lu", (unsigned long) gid);
		}
	}
	return 0;
}

#endif /* LINUX */

int
sys_setreuid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, %lu",
			(unsigned long) (uid_t) tcp->u_arg[0],
			(unsigned long) (uid_t) tcp->u_arg[1]);
	}
	return 0;
}

int
sys_setregid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, %lu",
			(unsigned long) (gid_t) tcp->u_arg[0],
			(unsigned long) (gid_t) tcp->u_arg[1]);
	}
	return 0;
}

#ifdef LINUX
int
sys_setresuid(tcp)
     struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("ruid %u, euid %u, suid %u",
				(uid_t) tcp->u_arg[0],
				(uid_t) tcp->u_arg[1],
				(uid_t) tcp->u_arg[2]);
	}
	return 0;
}
int
sys_setresgid(tcp)
     struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("rgid %u, egid %u, sgid %u",
				(uid_t) tcp->u_arg[0],
				(uid_t) tcp->u_arg[1],
				(uid_t) tcp->u_arg[2]);
	}
	return 0;
}

#endif /* LINUX */

int
sys_setgroups(tcp)
struct tcb *tcp;
{
	int i, len;
	GETGROUPS_T *gidset;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%u, ", len);
		if (len <= 0) {
			tprintf("[]");
			return 0;
		}
		gidset = (GETGROUPS_T *) malloc(len * sizeof(GETGROUPS_T));
		if (gidset == NULL) {
			fprintf(stderr, "sys_setgroups: out of memory\n");
			return -1;
		}
		if (!verbose(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umoven(tcp, tcp->u_arg[1],
		    len * sizeof(GETGROUPS_T), (char *) gidset) < 0)
			tprintf("[?]");
		else {
			tprintf("[");
			for (i = 0; i < len; i++)
				tprintf("%s%lu", i ? ", " : "",
					(unsigned long) gidset[i]);
			tprintf("]");
		}
		free((char *) gidset);
	}
	return 0;
}

int
sys_getgroups(tcp)
struct tcb *tcp;
{
	int i, len;
	GETGROUPS_T *gidset;

	if (entering(tcp)) {
		len = tcp->u_arg[0];
		tprintf("%u, ", len);
	} else {
		len = tcp->u_rval;
		if (len <= 0) {
			tprintf("[]");
			return 0;
		}
		gidset = (GETGROUPS_T *) malloc(len * sizeof(GETGROUPS_T));
		if (gidset == NULL) {
			fprintf(stderr, "sys_getgroups: out of memory\n");
			return -1;
		}
		if (!tcp->u_arg[1])
			tprintf("NULL");
		else if (!verbose(tcp) || tcp->u_arg[0] == 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umoven(tcp, tcp->u_arg[1],
		    len * sizeof(GETGROUPS_T), (char *) gidset) < 0)
			tprintf("[?]");
		else {
			tprintf("[");
			for (i = 0; i < len; i++)
				tprintf("%s%lu", i ? ", " : "",
					(unsigned long) gidset[i]);
			tprintf("]");
		}
		free((char *)gidset);
	}
	return 0;
}

int
sys_setpgrp(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
#ifndef SVR4
		tprintf("%lu, %lu", tcp->u_arg[0], tcp->u_arg[1]);
#endif /* !SVR4 */
	}
	return 0;
}

int
sys_getpgrp(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
#ifndef SVR4
		tprintf("%lu", tcp->u_arg[0]);
#endif /* !SVR4 */
	}
	return 0;
}

int
sys_getsid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
	}
	return 0;
}

int
sys_setsid(tcp)
struct tcb *tcp;
{
	return 0;
}

int
sys_getpgid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu", tcp->u_arg[0]);
	}
	return 0;
}

int
sys_setpgid(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%lu, %lu", tcp->u_arg[0], tcp->u_arg[1]);
	}
	return 0;
}

void
fake_execve(tcp, program, argv, envp)
struct tcb *tcp;
char *program;
char *argv[];
char *envp[];
{
	int i;

#ifdef ARM
	if (!(qual_flags[SYS_execve - __NR_SYSCALL_BASE] & QUAL_TRACE))
		return;
#else
	if (!(qual_flags[SYS_execve] & QUAL_TRACE))
		return;
#endif /* !ARM */
	printleader(tcp);
	tprintf("execve(");
	string_quote(program);
	tprintf(", [");
	for (i = 0; argv[i] != NULL; i++) {
		if (i != 0)
			tprintf(", ");
		string_quote(argv[i]);
	}
	for (i = 0; envp[i] != NULL; i++)
		;
	tprintf("], [/* %d var%s */]) ", i, (i != 1) ? "s" : "");
	tabto(acolumn);
	tprintf("= 0");
	printtrailer(tcp);
}

static void
printargv(tcp, addr)
struct tcb *tcp;
long addr;
{
	char *cp;
	char *sep;
	int max = max_strlen / 2;

	for (sep = ""; --max >= 0; sep = ", ") {
		if (!abbrev(tcp))
			max++;
		if (umove(tcp, addr, &cp) < 0) {
			tprintf("%#lx", addr);
			return;
		}
		if (cp == 0)
			break;
		tprintf(sep);
		printstr(tcp, (long) cp, -1);
		addr += sizeof(char *);
	}
	if (cp)
		tprintf(", ...");
}

static void
printargc(fmt, tcp, addr)
char *fmt;
struct tcb *tcp;
long addr;
{
	int count;
	char *cp;

	for (count = 0; umove(tcp, addr, &cp) >= 0 && cp != NULL; count++) {
		addr += sizeof(char *);
	}
	tprintf(fmt, count, count == 1 ? "" : "s");
}

int
sys_execv(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[1]);
#if 0
		else if (abbrev(tcp))
			printargc(", [/* %d arg%s */]", tcp, tcp->u_arg[1]);
#endif
		else {
			tprintf(", [");
			printargv(tcp, tcp->u_arg[1]);
			tprintf("]");
		}
	}
	return 0;
}

int
sys_execve(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[1]);
#if 0
		else if (abbrev(tcp))
			printargc(", [/* %d arg%s */]", tcp, tcp->u_arg[1]);
#endif
		else {
			tprintf(", [");
			printargv(tcp, tcp->u_arg[1]);
			tprintf("]");
		}
		if (!verbose(tcp))
			tprintf(", %#lx", tcp->u_arg[2]);
		else if (abbrev(tcp))
			printargc(", [/* %d var%s */]", tcp, tcp->u_arg[2]);
		else {
			tprintf(", [");
			printargv(tcp, tcp->u_arg[2]);
			tprintf("]");
		}
	}
#ifdef LINUX
#if defined(ALPHA) || defined(SPARC) || defined(POWERPC)
	tcp->flags |= TCB_WAITEXECVE;
#endif /* ALPHA || SPARC || POWERPC */
#endif /* LINUX */
	return 0;
}

int
internal_exec(tcp)
struct tcb *tcp;
{
#ifdef SUNOS4
	if (exiting(tcp) && !syserror(tcp) && followfork)
		fixvfork(tcp);
#endif /* SUNOS4 */
	return 0;
}

#ifdef LINUX
#ifndef __WCLONE
#define __WCLONE	0x8000000
#endif
#endif /* LINUX */

static struct xlat wait4_options[] = {
	{ WNOHANG,	"WNOHANG"	},
#ifndef WSTOPPED
	{ WUNTRACED,	"WUNTRACED"	},
#endif
#ifdef WEXITED
	{ WEXITED,	"WEXITED"	},
#endif
#ifdef WTRAPPED
	{ WTRAPPED,	"WTRAPPED"	},
#endif
#ifdef WSTOPPED
	{ WSTOPPED,	"WSTOPPED"	},
#endif
#ifdef WCONTINUED
	{ WCONTINUED,	"WCONTINUED"	},
#endif
#ifdef WNOWAIT
	{ WNOWAIT,	"WNOWAIT"	},
#endif
#ifdef __WCLONE
	{ __WCLONE,	"__WCLONE"	},
#endif
	{ 0,		NULL		},
};

static int
printstatus(status)
int status;
{
	int exited = 0;

	/*
	 * Here is a tricky presentation problem.  This solution
	 * is still not entirely satisfactory but since there
	 * are no wait status constructors it will have to do.
	 */
	if (WIFSTOPPED(status))
		tprintf("[WIFSTOPPED(s) && WSTOPSIG(s) == %s]",
			signame(WSTOPSIG(status)));
	else if WIFSIGNALED(status)
		tprintf("[WIFSIGNALED(s) && WTERMSIG(s) == %s%s]",
			signame(WTERMSIG(status)),
			WCOREDUMP(status) ? " && WCOREDUMP(s)" : "");
	else if WIFEXITED(status) {
		tprintf("[WIFEXITED(s) && WEXITSTATUS(s) == %d]",
			WEXITSTATUS(status));
		exited = 1;
	}
	else
		tprintf("[%#x]", status);
	return exited;
}

static int
printwaitn(tcp, n)
struct tcb *tcp;
int n;
{
	int status;
	int exited = 0;

	if (entering(tcp)) {
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		/* status */
		if (!tcp->u_arg[1])
			tprintf("NULL");
		else if (syserror(tcp) || tcp->u_rval == 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else if (umove(tcp, tcp->u_arg[1], &status) < 0)
			tprintf("[?]");
		else
			exited = printstatus(status);
		/* options */
		tprintf(", ");
		if (!printflags(wait4_options, tcp->u_arg[2]))
			tprintf("0");
		if (n == 4) {
			tprintf(", ");
			/* usage */
			if (!tcp->u_arg[3])
				tprintf("NULL");
#ifdef LINUX
			else if (tcp->u_rval > 0)
				printrusage(tcp, tcp->u_arg[3]);
#endif /* LINUX */
#ifdef SUNOS4
			else if (tcp->u_rval > 0 && exited)
				printrusage(tcp, tcp->u_arg[3]);
#endif /* SUNOS4 */
			else
				tprintf("%#lx", tcp->u_arg[3]);
		}
	}
	return 0;
}

int
internal_wait(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		/* WTA: fix bug with hanging children */
		if (!(tcp->u_arg[2] & WNOHANG) && tcp->nchildren > 0) {
			/* There are traced children */
			tcp->flags |= TCB_SUSPENDED;
			tcp->waitpid = tcp->u_arg[0];
		}
	}
	return 0;
}

#ifdef SVR4

int
sys_wait(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		/* The library wrapper stuffs this into the user variable. */
		if (!syserror(tcp))
			printstatus(getrval2(tcp));
	}
	return 0;
}

#endif /* SVR4 */

int
sys_waitpid(tcp)
struct tcb *tcp;
{
	return printwaitn(tcp, 3);
}

int
sys_wait4(tcp)
struct tcb *tcp;
{
	return printwaitn(tcp, 4);
}

#ifdef SVR4

static struct xlat waitid_types[] = {
	{ P_PID,	"P_PID"		},
	{ P_PPID,	"P_PPID"	},
	{ P_PGID,	"P_PGID"	},
	{ P_SID,	"P_SID"		},
	{ P_CID,	"P_CID"		},
	{ P_UID,	"P_UID"		},
	{ P_GID,	"P_GID"		},
	{ P_ALL,	"P_ALL"		},
#ifdef P_LWPID
	{ P_LWPID,	"P_LWPID"	},
#endif
	{ 0,		NULL		},
};

static struct xlat siginfo_codes[] = {
#ifdef SI_NOINFO
	{ SI_NOINFO,	"SI_NOINFO"	},
#endif
#ifdef SI_USER
	{ SI_USER,	"SI_USER"	},
#endif
#ifdef SI_LWP
	{ SI_LWP,	"SI_LWP"	},
#endif
#ifdef SI_QUEUE
	{ SI_QUEUE,	"SI_QUEUE"	},
#endif
#ifdef SI_TIMER
	{ SI_TIMER,	"SI_TIMER"	},
#endif
#ifdef SI_ASYNCIO
	{ SI_ASYNCIO,	"SI_ASYNCIO"	},
#endif
#ifdef SI_MESGQ
	{ SI_MESGQ,	"SI_MESGQ"	},
#endif
	{ 0,		NULL		},
};

static struct xlat sigtrap_codes[] = {
	{ TRAP_BRKPT,	"TRAP_BRKPT"	},
	{ TRAP_TRACE,	"TRAP_TRACE"	},
	{ 0,		NULL		},
};

static struct xlat sigcld_codes[] = {
	{ CLD_EXITED,	"CLD_EXITED"	},
	{ CLD_KILLED,	"CLD_KILLED"	},
	{ CLD_DUMPED,	"CLD_DUMPED"	},
	{ CLD_TRAPPED,	"CLD_TRAPPED"	},
	{ CLD_STOPPED,	"CLD_STOPPED"	},
	{ CLD_CONTINUED,"CLD_CONTINUED"	},
	{ 0,		NULL		},
};

static struct xlat sigpoll_codes[] = {
	{ POLL_IN,	"POLL_IN"	},
	{ POLL_OUT,	"POLL_OUT"	},
	{ POLL_MSG,	"POLL_MSG"	},
	{ POLL_ERR,	"POLL_ERR"	},
	{ POLL_PRI,	"POLL_PRI"	},
	{ POLL_HUP,	"POLL_HUP"	},
	{ 0,		NULL		},
};

static struct xlat sigprof_codes[] = {
#ifdef PROF_SIG
	{ PROF_SIG,	"PROF_SIG"	},
#endif
	{ 0,		NULL		},
};

static struct xlat sigill_codes[] = {
	{ ILL_ILLOPC,	"ILL_ILLOPC"	},
	{ ILL_ILLOPN,	"ILL_ILLOPN"	},
	{ ILL_ILLADR,	"ILL_ILLADR"	},
	{ ILL_ILLTRP,	"ILL_ILLTRP"	},
	{ ILL_PRVOPC,	"ILL_PRVOPC"	},
	{ ILL_PRVREG,	"ILL_PRVREG"	},
	{ ILL_COPROC,	"ILL_COPROC"	},
	{ ILL_BADSTK,	"ILL_BADSTK"	},
	{ 0,		NULL		},
};

static struct xlat sigemt_codes[] = {
#ifdef EMT_TAGOVF
	{ EMT_TAGOVF,	"EMT_TAGOVF"	},
#endif
	{ 0,		NULL		},
};

static struct xlat sigfpe_codes[] = {
	{ FPE_INTDIV,	"FPE_INTDIV"	},
	{ FPE_INTOVF,	"FPE_INTOVF"	},
	{ FPE_FLTDIV,	"FPE_FLTDIV"	},
	{ FPE_FLTOVF,	"FPE_FLTOVF"	},
	{ FPE_FLTUND,	"FPE_FLTUND"	},
	{ FPE_FLTRES,	"FPE_FLTRES"	},
	{ FPE_FLTINV,	"FPE_FLTINV"	},
	{ FPE_FLTSUB,	"FPE_FLTSUB"	},
	{ 0,		NULL		},
};

static struct xlat sigsegv_codes[] = {
	{ SEGV_MAPERR,	"SEGV_MAPERR"	},
	{ SEGV_ACCERR,	"SEGV_ACCERR"	},
	{ 0,		NULL		},
};

static struct xlat sigbus_codes[] = {
	{ BUS_ADRALN,	"BUS_ADRALN"	},
	{ BUS_ADRERR,	"BUS_ADRERR"	},
	{ BUS_OBJERR,	"BUS_OBJERR"	},
	{ 0,		NULL		},
};

void
printsiginfo(sip)
siginfo_t *sip;
{
	char *code;

	tprintf("{si_signo=");
	printsignal(sip->si_signo);
	code = xlookup(siginfo_codes, sip->si_code);
	if (!code) {
		switch (sip->si_signo) {
		case SIGTRAP:
			code = xlookup(sigtrap_codes, sip->si_code);
			break;
		case SIGCHLD:
			code = xlookup(sigcld_codes, sip->si_code);
			break;
		case SIGPOLL:
			code = xlookup(sigpoll_codes, sip->si_code);
			break;
		case SIGPROF:
			code = xlookup(sigprof_codes, sip->si_code);
			break;
		case SIGILL:
			code = xlookup(sigill_codes, sip->si_code);
			break;
		case SIGEMT:
			code = xlookup(sigemt_codes, sip->si_code);
			break;
		case SIGFPE:
			code = xlookup(sigfpe_codes, sip->si_code);
			break;
		case SIGSEGV:
			code = xlookup(sigsegv_codes, sip->si_code);
			break;
		case SIGBUS:
			code = xlookup(sigbus_codes, sip->si_code);
			break;
		}
	}
	if (code)
		tprintf(", si_code=%s", code);
	else
		tprintf(", si_code=%#x", sip->si_code);
#ifdef SI_NOINFO
	if (sip->si_code != SI_NOINFO) {
#endif
		if (sip->si_errno) {
			if (sip->si_errno < 0 || sip->si_errno >= nerrnos)
				tprintf(", si_errno=%d", sip->si_errno);
			else
				tprintf(", si_errno=%s",
					errnoent[sip->si_errno]);
		}
		if (SI_FROMUSER(sip)) {
#ifdef SI_QUEUE
			tprintf(", si_pid=%ld, si_uid=%ld",
				sip->si_pid, sip->si_uid);
			switch (sip->si_code) {
			case SI_QUEUE:
#ifdef SI_TIMER
			case SI_TIMER:
#endif /* SI_QUEUE */
			case SI_ASYNCIO:
#ifdef SI_MESGQ
			case SI_MESGQ:
#endif /* SI_MESGQ */
				tprintf(", si_value=%d",
					sip->si_value.sival_int);
				break;
			}
#endif /* SI_QUEUE */
		}
		else {
			switch (sip->si_signo) {
			case SIGCHLD:
				tprintf(", si_pid=%ld, si_status=",
					sip->si_pid);
				if (sip->si_code == CLD_EXITED)
					tprintf("%d", sip->si_status);
				else
					printsignal(sip->si_status);
				break;
			case SIGILL: case SIGFPE:
			case SIGSEGV: case SIGBUS:
				tprintf(", si_addr=%#lx",
					(unsigned long) sip->si_addr);
				break;
			case SIGPOLL:
				switch (sip->si_code) {
				case POLL_IN: case POLL_OUT: case POLL_MSG:
					tprintf(", si_band=%ld",
						(long) sip->si_band);
					break;
				}
				break;
			}
		}
		tprintf(", ...");
#ifdef SI_NOINFO
	}
#endif
	tprintf("}");
}

int
sys_waitid(tcp)
struct tcb *tcp;
{
	siginfo_t si;
	int exited;

	if (entering(tcp)) {
		printxval(waitid_types, tcp->u_arg[0], "P_???");
		tprintf(", %ld, ", tcp->u_arg[1]);
		if (tcp->nchildren > 0) {
			/* There are traced children */
			tcp->flags |= TCB_SUSPENDED;
			tcp->waitpid = tcp->u_arg[0];
		}
	}
	else {
		/* siginfo */
		exited = 0;
		if (!tcp->u_arg[2])
			tprintf("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else if (umove(tcp, tcp->u_arg[2], &si) < 0)
			tprintf("{???}");
		else
			printsiginfo(&si);
		/* options */
		tprintf(", ");
		if (!printflags(wait4_options, tcp->u_arg[3]))
			tprintf("0");
	}
	return 0;
}

#endif /* SVR4 */

int
sys_alarm(tcp)
struct tcb *tcp;
{
	if (entering(tcp))
		tprintf("%lu", tcp->u_arg[0]);
	return 0;
}

int
sys_uname(tcp)
struct tcb *tcp;
{
	struct utsname uname;

	if (exiting(tcp)) {
		if (syserror(tcp) || !verbose(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (umove(tcp, tcp->u_arg[0], &uname) < 0)
			tprintf("{...}");
		else if (!abbrev(tcp)) {

			tprintf("{sysname=\"%s\", nodename=\"%s\", ",
				uname.sysname, uname.nodename);
			tprintf("release=\"%s\", version=\"%s\", ",
				uname.release, uname.version);
			tprintf("machine=\"%s\"", uname.machine);
#ifdef LINUX
#ifndef __GLIBC__
			tprintf(", domainname=\"%s\"", uname.domainname);
#endif /* __GLIBC__ */
#endif /* LINUX */
			tprintf("}");
		}
		else
			tprintf("{sys=\"%s\", node=\"%s\", ...}",
				uname.sysname, uname.nodename);
	}
	return 0;
}

#ifndef SVR4

static struct xlat ptrace_cmds[] = {
	{ PTRACE_TRACEME,	"PTRACE_TRACEME"	},
	{ PTRACE_PEEKTEXT,	"PTRACE_PEEKTEXT",	},
	{ PTRACE_PEEKDATA,	"PTRACE_PEEKDATA",	},
	{ PTRACE_PEEKUSER,	"PTRACE_PEEKUSER",	},
	{ PTRACE_POKETEXT,	"PTRACE_POKETEXT",	},
	{ PTRACE_POKEDATA,	"PTRACE_POKEDATA",	},
	{ PTRACE_POKEUSER,	"PTRACE_POKEUSER",	},
	{ PTRACE_CONT,		"PTRACE_CONT"		},
	{ PTRACE_KILL,		"PTRACE_KILL"		},
	{ PTRACE_SINGLESTEP,	"PTRACE_SINGLESTEP"	},
	{ PTRACE_ATTACH,	"PTRACE_ATTACH"		},
	{ PTRACE_DETACH,	"PTRACE_DETACH"		},
#ifdef SUNOS4
	{ PTRACE_GETREGS,	"PTRACE_GETREGS"	},
	{ PTRACE_SETREGS,	"PTRACE_SETREGS"	},
	{ PTRACE_GETFPREGS,	"PTRACE_GETFPREGS",	},
	{ PTRACE_SETFPREGS,	"PTRACE_SETFPREGS",	},
	{ PTRACE_READDATA,	"PTRACE_READDATA"	},
	{ PTRACE_WRITEDATA,	"PTRACE_WRITEDATA"	},
	{ PTRACE_READTEXT,	"PTRACE_READTEXT"	},
	{ PTRACE_WRITETEXT,	"PTRACE_WRITETEXT"	},
	{ PTRACE_GETFPAREGS,	"PTRACE_GETFPAREGS"	},
	{ PTRACE_SETFPAREGS,	"PTRACE_SETFPAREGS"	},
#ifdef SPARC
	{ PTRACE_GETWINDOW,	"PTRACE_GETWINDOW"	},
	{ PTRACE_SETWINDOW,	"PTRACE_SETWINDOW"	},
#else /* !SPARC */
	{ PTRACE_22,		"PTRACE_PTRACE_22"	},
	{ PTRACE_23,		"PTRACE_PTRACE_23"	},
#endif /* !SPARC */
#endif /* SUNOS4 */
	{ PTRACE_SYSCALL,	"PTRACE_SYSCALL"	},
#ifdef SUNOS4
	{ PTRACE_DUMPCORE,	"PTRACE_DUMPCORE"	},
#ifdef I386
	{ PTRACE_SETWRBKPT,	"PTRACE_SETWRBKPT"	},
	{ PTRACE_SETACBKPT,	"PTRACE_SETACBKPT"	},
	{ PTRACE_CLRDR7,	"PTRACE_CLRDR7"		},
#else /* !I386 */
	{ PTRACE_26,		"PTRACE_26"		},
	{ PTRACE_27,		"PTRACE_27"		},
	{ PTRACE_28,		"PTRACE_28"		},
#endif /* !I386 */
	{ PTRACE_GETUCODE,	"PTRACE_GETUCODE"	},
#endif /* SUNOS4 */
	{ 0,			NULL			},
};

#ifndef SUNOS4_KERNEL_ARCH_KLUDGE
static
#endif /* !SUNOS4_KERNEL_ARCH_KLUDGE */
struct xlat struct_user_offsets[] = {
#ifdef LINUX
#ifdef SPARC
	/* XXX No support for these offsets yet. */
#elif defined(POWERPC)
	{ 4*PT_R0,		"4*PT_R0"	},
	{ 4*PT_R1,		"4*PT_R1"	},
	{ 4*PT_R2,		"4*PT_R2"	},
	{ 4*PT_R3,		"4*PT_R3"	},
	{ 4*PT_R4,		"4*PT_R4"	},
	{ 4*PT_R5,		"4*PT_R5"	},
	{ 4*PT_R6,		"4*PT_R6"	},
	{ 4*PT_R7,		"4*PT_R7"	},
	{ 4*PT_R8,		"4*PT_R8"	},
	{ 4*PT_R9,		"4*PT_R9"	},
	{ 4*PT_R10,		"4*PT_R10"	},
	{ 4*PT_R11,		"4*PT_R11"	},
	{ 4*PT_R12,		"4*PT_R12"	},
	{ 4*PT_R13,		"4*PT_R13"	},
	{ 4*PT_R14,		"4*PT_R14"	},
	{ 4*PT_R15,		"4*PT_R15"	},
	{ 4*PT_R16,		"4*PT_R16"	},
	{ 4*PT_R17,		"4*PT_R17"	},
	{ 4*PT_R18,		"4*PT_R18"	},
	{ 4*PT_R19,		"4*PT_R19"	},
	{ 4*PT_R20,		"4*PT_R20"	},
	{ 4*PT_R21,		"4*PT_R21"	},
	{ 4*PT_R22,		"4*PT_R22"	},
	{ 4*PT_R23,		"4*PT_R23"	},
	{ 4*PT_R24,		"4*PT_R24"	},
	{ 4*PT_R25,		"4*PT_R25"	},
	{ 4*PT_R26,		"4*PT_R26"	},
	{ 4*PT_R27,		"4*PT_R27"	},
	{ 4*PT_R28,		"4*PT_R28"	},
	{ 4*PT_R29,		"4*PT_R29"	},
	{ 4*PT_R30,		"4*PT_R30"	},
	{ 4*PT_R31,		"4*PT_R31"	},
	{ 4*PT_NIP,		"4*PT_NIP"	},
	{ 4*PT_MSR,		"4*PT_MSR"	},
	{ 4*PT_ORIG_R3,	 "4*PT_ORIG_R3"	},
	{ 4*PT_CTR,		"4*PT_CTR"	},
	{ 4*PT_LNK,		"4*PT_LNK"	},
	{ 4*PT_XER,		"4*PT_XER"	},
	{ 4*PT_CCR,		"4*PT_CCR"	},
	{ 4*PT_FPR0,	"4*PT_FPR0"	},
#else	
#ifdef ALPHA
	{ 0,		"r0"					},
	{ 1,		"r1"					},
	{ 2,		"r2"					},
	{ 3,		"r3"					},
	{ 4,		"r4"					},
	{ 5,		"r5"					},
	{ 6,		"r6"					},
	{ 7,		"r7"					},
	{ 8,		"r8"					},
	{ 9,		"r9"					},
	{ 10,		"r10"					},
	{ 11,		"r11"					},
	{ 12,		"r12"					},
	{ 13,		"r13"					},
	{ 14,		"r14"					},
	{ 15,		"r15"					},
	{ 16,		"r16"					},
	{ 17,		"r17"					},
	{ 18,		"r18"					},
	{ 19,		"r19"					},
	{ 20,		"r20"					},
	{ 21,		"r21"					},
	{ 22,		"r22"					},
	{ 23,		"r23"					},
	{ 24,		"r24"					},
	{ 25,		"r25"					},
	{ 26,		"r26"					},
	{ 27,		"r27"					},
	{ 28,		"r28"					},
	{ 29,		"gp"					},
	{ 30,		"fp"					},
	{ 31,		"zero"					},
	{ 32,		"fp0"					},
	{ 33,		"fp"					},
	{ 34,		"fp2"					},
	{ 35,		"fp3"					},
	{ 36,		"fp4"					},
	{ 37,		"fp5"					},
	{ 38,		"fp6"					},
	{ 39,		"fp7"					},
	{ 40,		"fp8"					},
	{ 41,		"fp9"					},
	{ 42,		"fp10"					},
	{ 43,		"fp11"					},
	{ 44,		"fp12"					},
	{ 45,		"fp13"					},
	{ 46,		"fp14"					},
	{ 47,		"fp15"					},
	{ 48,		"fp16"					},
	{ 49,		"fp17"					},
	{ 50,		"fp18"					},
	{ 51,		"fp19"					},
	{ 52,		"fp20"					},
	{ 53,		"fp21"					},
	{ 54,		"fp22"					},
	{ 55,		"fp23"					},
	{ 56,		"fp24"					},
	{ 57,		"fp25"					},
	{ 58,		"fp26"					},
	{ 59,		"fp27"					},
	{ 60,		"fp28"					},
	{ 61,		"fp29"					},
	{ 62,		"fp30"					},
	{ 63,		"fp31"					},
	{ 64,		"pc"					},
#else /* !ALPHA */
#ifdef I386
	{ 4*EBX,		"4*EBX"					},
	{ 4*ECX,		"4*ECX"					},
	{ 4*EDX,		"4*EDX"					},
	{ 4*ESI,		"4*ESI"					},
	{ 4*EDI,		"4*EDI"					},
	{ 4*EBP,		"4*EBP"					},
	{ 4*EAX,		"4*EAX"					},
	{ 4*DS,			"4*DS"					},
	{ 4*ES,			"4*ES"					},
	{ 4*FS,			"4*FS"					},
	{ 4*GS,			"4*GS"					},
	{ 4*ORIG_EAX,		"4*ORIG_EAX"				},
	{ 4*EIP,		"4*EIP"					},
	{ 4*CS,			"4*CS"					},
	{ 4*EFL,		"4*EFL"					},
	{ 4*UESP,		"4*UESP"				},
	{ 4*SS,			"4*SS"					},
#else /* !I386 */
#ifdef M68K
	{ 4*PT_D1,		"4*PT_D1"				},
	{ 4*PT_D2,		"4*PT_D2"				},
	{ 4*PT_D3,		"4*PT_D3"				},
	{ 4*PT_D4,		"4*PT_D4"				},
	{ 4*PT_D5,		"4*PT_D5"				},
	{ 4*PT_D6,		"4*PT_D6"				},
	{ 4*PT_D7,		"4*PT_D7"				},
	{ 4*PT_A0,		"4*PT_A0"				},
	{ 4*PT_A1,		"4*PT_A1"				},
	{ 4*PT_A2,		"4*PT_A2"				},
	{ 4*PT_A3,		"4*PT_A3"				},
	{ 4*PT_A4,		"4*PT_A4"				},
	{ 4*PT_A5,		"4*PT_A5"				},
	{ 4*PT_A6,		"4*PT_A6"				},
	{ 4*PT_D0,		"4*PT_D0"				},
	{ 4*PT_USP,		"4*PT_USP"				},
	{ 4*PT_ORIG_D0,		"4*PT_ORIG_D0"				},
	{ 4*PT_SR,		"4*PT_SR"				},
	{ 4*PT_PC,		"4*PT_PC"				},
#endif /* M68K */
#endif /* !I386 */
	{ uoff(u_fpvalid),	"offsetof(struct user, u_fpvalid)"	},
#ifdef I386
	{ uoff(i387),		"offsetof(struct user, i387)"		},
#else /* !I386 */
#ifdef M68K
	{ uoff(m68kfp),		"offsetof(struct user, m68kfp)"		},
#endif /* M68K */
#endif /* !I386 */
	{ uoff(u_tsize),	"offsetof(struct user, u_tsize)"	},
	{ uoff(u_dsize),	"offsetof(struct user, u_dsize)"	},
	{ uoff(u_ssize),	"offsetof(struct user, u_ssize)"	},
	{ uoff(start_code),	"offsetof(struct user, start_code)"	},
	{ uoff(start_stack),	"offsetof(struct user, start_stack)"	},
	{ uoff(signal),		"offsetof(struct user, signal)"		},
	{ uoff(reserved),	"offsetof(struct user, reserved)"	},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
#ifndef ARM
	{ uoff(u_fpstate),	"offsetof(struct user, u_fpstate)"	},
#endif
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
#ifdef I386
	{ uoff(u_debugreg),	"offsetof(struct user, u_debugreg)"	},
#endif /* I386 */
#endif /* !ALPHA */
#endif /* !POWERPC/!SPARC */
#endif /* LINUX */
#ifdef SUNOS4
	{ uoff(u_pcb),		"offsetof(struct user, u_pcb)"		},
	{ uoff(u_procp),	"offsetof(struct user, u_procp)"	},
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
	{ uoff(u_comm[0]),	"offsetof(struct user, u_comm[0])"	},
	{ uoff(u_arg[0]),	"offsetof(struct user, u_arg[0])"	},
	{ uoff(u_ap),		"offsetof(struct user, u_ap)"		},
	{ uoff(u_qsave),	"offsetof(struct user, u_qsave)"	},
	{ uoff(u_rval1),	"offsetof(struct user, u_rval1)"	},
	{ uoff(u_rval2),	"offsetof(struct user, u_rval2)"	},
	{ uoff(u_error),	"offsetof(struct user, u_error)"	},
	{ uoff(u_eosys),	"offsetof(struct user, u_eosys)"	},
	{ uoff(u_ssave),	"offsetof(struct user, u_ssave)"	},
	{ uoff(u_signal[0]),	"offsetof(struct user, u_signal)"	},
	{ uoff(u_sigmask[0]),	"offsetof(struct user, u_sigmask)"	},
	{ uoff(u_sigonstack),	"offsetof(struct user, u_sigonstack)"	},
	{ uoff(u_sigintr),	"offsetof(struct user, u_sigintr)"	},
	{ uoff(u_sigreset),	"offsetof(struct user, u_sigreset)"	},
	{ uoff(u_oldmask),	"offsetof(struct user, u_oldmask)"	},
	{ uoff(u_code),		"offsetof(struct user, u_code)"		},
	{ uoff(u_addr),		"offsetof(struct user, u_addr)"		},
	{ uoff(u_sigstack),	"offsetof(struct user, u_sigstack)"	},
	{ uoff(u_ofile),	"offsetof(struct user, u_ofile)"	},
	{ uoff(u_pofile),	"offsetof(struct user, u_pofile)"	},
	{ uoff(u_ofile_arr[0]),	"offsetof(struct user, u_ofile_arr[0])"	},
	{ uoff(u_pofile_arr[0]),"offsetof(struct user, u_pofile_arr[0])"},
	{ uoff(u_lastfile),	"offsetof(struct user, u_lastfile)"	},
	{ uoff(u_cwd),		"offsetof(struct user, u_cwd)"		},
	{ uoff(u_cdir),		"offsetof(struct user, u_cdir)"		},
	{ uoff(u_rdir),		"offsetof(struct user, u_rdir)"		},
	{ uoff(u_cmask),	"offsetof(struct user, u_cmask)"	},
	{ uoff(u_ru),		"offsetof(struct user, u_ru)"		},
	{ uoff(u_cru),		"offsetof(struct user, u_cru)"		},
	{ uoff(u_timer[0]),	"offsetof(struct user, u_timer[0])"	},
	{ uoff(u_XXX[0]),	"offsetof(struct user, u_XXX[0])"	},
	{ uoff(u_ioch),		"offsetof(struct user, u_ioch)"		},
	{ uoff(u_start),	"offsetof(struct user, u_start)"	},
	{ uoff(u_acflag),	"offsetof(struct user, u_acflag)"	},
	{ uoff(u_prof.pr_base),	"offsetof(struct user, u_prof.pr_base)"	},
	{ uoff(u_prof.pr_size),	"offsetof(struct user, u_prof.pr_size)"	},
	{ uoff(u_prof.pr_off),	"offsetof(struct user, u_prof.pr_off)"	},
	{ uoff(u_prof.pr_scale),"offsetof(struct user, u_prof.pr_scale)"},
	{ uoff(u_rlimit[0]),	"offsetof(struct user, u_rlimit)"	},
	{ uoff(u_exdata.Ux_A),	"offsetof(struct user, u_exdata.Ux_A)"	},
	{ uoff(u_exdata.ux_shell[0]),"offsetof(struct user, u_exdata.ux_shell[0])"},
	{ uoff(u_lofault),	"offsetof(struct user, u_lofault)"	},
#endif /* SUNOS4 */
	{ sizeof(struct user),	"sizeof(struct user)"			},
	{ 0,			NULL					},
};

int
sys_ptrace(tcp)
struct tcb *tcp;
{
	char *cmd;
	struct xlat *x;
	long addr;

	cmd = xlookup(ptrace_cmds, tcp->u_arg[0]);
	if (!cmd)
		cmd = "PTRACE_???";
	if (entering(tcp)) {
		tprintf("%s, %lu, ", cmd, tcp->u_arg[1]);
		addr = tcp->u_arg[2];
		if (tcp->u_arg[0] == PTRACE_PEEKUSER
			|| tcp->u_arg[0] == PTRACE_POKEUSER) {
			for (x = struct_user_offsets; x->str; x++) {
				if (x->val >= addr)
					break;
			}
			if (!x->str)
				tprintf("%#lx, ", addr);
			else if (x->val > addr && x != struct_user_offsets) {
				x--;
				tprintf("%s + %ld, ", x->str, addr - x->val);
			}
			else
				tprintf("%s, ", x->str);
		}
		else
			tprintf("%#lx, ", tcp->u_arg[2]);
#ifdef LINUX
		switch (tcp->u_arg[0]) {
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			break;
		case PTRACE_CONT:
		case PTRACE_SINGLESTEP:
		case PTRACE_SYSCALL:
		case PTRACE_DETACH:
			printsignal(tcp->u_arg[3]);
			break;
		default:
			tprintf("%#lx", tcp->u_arg[3]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			printnum(tcp, tcp->u_arg[3], "%#x");
			break;
		}
	}
#endif /* LINUX */
#ifdef SUNOS4
		if (tcp->u_arg[0] == PTRACE_WRITEDATA ||
			tcp->u_arg[0] == PTRACE_WRITETEXT) {
			tprintf("%lu, ", tcp->u_arg[3]);
			printstr(tcp, tcp->u_arg[4], tcp->u_arg[3]);
		} else if (tcp->u_arg[0] != PTRACE_READDATA &&
				tcp->u_arg[0] != PTRACE_READTEXT) {
			tprintf("%#lx", tcp->u_arg[3]);
		}
	} else {
		if (tcp->u_arg[0] == PTRACE_READDATA ||
			tcp->u_arg[0] == PTRACE_READTEXT) {
			tprintf("%lu, ", tcp->u_arg[3]);
			printstr(tcp, tcp->u_arg[4], tcp->u_arg[3]);
		}
	}
#endif /* SUNOS4 */
	return 0;
}

#endif /* !SVR4 */
