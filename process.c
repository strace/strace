/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>

 *
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

#ifdef FREEBSD
#include <sys/ptrace.h>
#endif

#if HAVE_ASM_REG_H
#ifdef SPARC
#  define fpq kernel_fpq
#  define fq kernel_fq
#  define fpu kernel_fpu
#endif
#include <asm/reg.h>
#ifdef SPARC
#  undef fpq
#  undef fq
#  undef fpu
#endif
#endif /* HAVE_ASM_REG_H */

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
#ifndef PTRACE_PEEKUSR
# define PTRACE_PEEKUSR PTRACE_PEEKUSER
#endif
#ifndef PTRACE_POKEUSR
# define PTRACE_POKEUSR PTRACE_POKEUSER
#endif
#endif

#ifdef HAVE_LINUX_PTRACE_H
#undef PTRACE_SYSCALL
#include <linux/ptrace.h>
#endif

#ifdef HAVE_LINUX_FUTEX_H
#include <linux/futex.h>
#endif
#if defined LINUX
# ifndef FUTEX_WAIT
#  define FUTEX_WAIT 0
# endif
# ifndef FUTEX_WAKE
#  define FUTEX_WAKE 1
# endif
# ifndef FUTEX_FD
#  define FUTEX_FD 2
# endif
#endif

#ifdef LINUX
#include <asm/posix_types.h>
#undef GETGROUPS_T
#define GETGROUPS_T __kernel_gid_t
#endif /* LINUX */

#if defined(LINUX) && defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

#ifdef HAVE_PRCTL
#include <sys/prctl.h>
#endif

#ifndef WCOREDUMP
#define WCOREDUMP(status) ((status) & 0200)
#endif

/* WTA: this was `&& !defined(LINUXSPARC)', this seems unneeded though? */
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
#ifdef PR_SET_PDEATHSIG
	{ PR_SET_PDEATHSIG,	"PR_SET_PDEATHSIG"	},
#endif
#ifdef PR_GET_PDEATHSIG
	{ PR_GET_PDEATHSIG,	"PR_GET_PDEATHSIG"	},
#endif
#ifdef PR_GET_UNALIGN
	{ PR_GET_UNALIGN,	"PR_GET_UNALIGN"	},
#endif
#ifdef PR_SET_UNALIGN
	{ PR_SET_UNALIGN,	"PR_SET_UNALIGN"	},
#endif
#ifdef PR_GET_KEEPCAPS
	{ PR_GET_KEEPCAPS,	"PR_GET_KEEP_CAPS"	},
#endif
#ifdef PR_SET_KEEPCAPS
	{ PR_SET_KEEPCAPS,	"PR_SET_KEEP_CAPS"	},
#endif
	{ 0,			NULL			},
};


const char *
unalignctl_string (unsigned int ctl)
{
	static char buf[16];

	switch (ctl) {
#ifdef PR_UNALIGN_NOPRINT
	      case PR_UNALIGN_NOPRINT:
		return "NOPRINT";
#endif
#ifdef PR_UNALIGN_SIGBUS
	      case PR_UNALIGN_SIGBUS:
		return "SIGBUS";
#endif
	      default:
		break;
	}
	sprintf(buf, "%x", ctl);
	return buf;
}


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
#ifdef PR_SET_DEATHSIG
		case PR_GET_PDEATHSIG:
			break;
#endif
#ifdef PR_SET_UNALIGN
		case PR_SET_UNALIGN:
			tprintf(", %s", unalignctl_string(tcp->u_arg[1]));
			break;
#endif
#ifdef PR_GET_UNALIGN
		case PR_GET_UNALIGN:
			tprintf(", %#lx", tcp->u_arg[1]);
			break;
#endif
		default:
			for (i = 1; i < tcp->u_nargs; i++)
				tprintf(", %#lx", tcp->u_arg[i]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
#ifdef PR_GET_PDEATHSIG
		case PR_GET_PDEATHSIG:
			for (i=1; i<tcp->u_nargs; i++)
				tprintf(", %#lx", tcp->u_arg[i]);
			break;
#endif
#ifdef PR_SET_UNALIGN
		case PR_SET_UNALIGN:
			break;
#endif
#ifdef PR_GET_UNALIGN
		case PR_GET_UNALIGN:
		{
			int ctl;

			umove(tcp, tcp->u_arg[1], &ctl);
			tcp->auxstr = unalignctl_string(ctl);
			return RVAL_STR;
		}
#endif
		default:
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
	if (entering(tcp)) {
		tcp->flags |= TCB_EXITING;
#ifdef __NR_exit_group
		if (tcp->scno == __NR_exit_group)
			tcp->flags |= TCB_GROUP_EXITING;
#endif
	}
	return 0;
}

/* TCP is creating a child we want to follow.
   If there will be space in tcbtab for it, set TCB_FOLLOWFORK and return 0.
   If not, clear TCB_FOLLOWFORK, print an error, and return 1.  */
static int
fork_tcb(struct tcb *tcp)
{
	if (nprocs == tcbtabsize) {
		/* Allocate some more TCBs and expand the table.
		   We don't want to relocate the TCBs because our
		   callers have pointers and it would be a pain.
		   So tcbtab is a table of pointers.  Since we never
		   free the TCBs, we allocate a single chunk of many.  */
		struct tcb **newtab = (struct tcb **)
			realloc(tcbtab, 2 * tcbtabsize * sizeof tcbtab[0]);
		struct tcb *newtcbs = (struct tcb *) calloc(tcbtabsize,
							    sizeof *newtcbs);
		int i;
		if (newtab == NULL || newtcbs == NULL) {
			if (newtab != NULL)
				free(newtab);
			tcp->flags &= ~TCB_FOLLOWFORK;
			fprintf(stderr, "sys_fork: tcb table full\n");
			return 1;
		}
		for (i = tcbtabsize; i < 2 * tcbtabsize; ++i)
			newtab[i] = &newtcbs[i - tcbtabsize];
		tcbtabsize *= 2;
		tcbtab = newtab;
	}

	tcp->flags |= TCB_FOLLOWFORK;
	return 0;
}

#ifdef USE_PROCFS

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

#if UNIXWARE > 2

int
sys_rfork(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf ("%ld", tcp->u_arg[0]);
	}
	else {
		if (getrval2(tcp)) {
			tcp->auxstr = "child process";
			return RVAL_UDECIMAL | RVAL_STR;
		}
	}
	return 0;
}

#endif

int
internal_fork(tcp)
struct tcb *tcp;
{
	struct tcb *tcpchild;

	if (exiting(tcp)) {
#ifdef SYS_rfork
		if (tcp->scno == SYS_rfork && !(tcp->u_arg[0]&RFPROC))
			return 0;
#endif
		if (getrval2(tcp))
			return 0;
		if (!followfork)
			return 0;
		if (fork_tcb(tcp))
			return 0;
		if (syserror(tcp))
			return 0;
		if ((tcpchild = alloctcb(tcp->u_rval)) == NULL) {
			fprintf(stderr, "sys_fork: tcb table full\n");
			return 0;
		}
		if (proc_open(tcpchild, 2) < 0)
		  	droptcb(tcpchild);
	}
	return 0;
}

#else /* !USE_PROCFS */

#ifdef LINUX

/* defines copied from linux/sched.h since we can't include that
 * ourselves (it conflicts with *lots* of libc includes)
 */
#define CSIGNAL         0x000000ff      /* signal mask to be sent at exit */
#define CLONE_VM        0x00000100      /* set if VM shared between processes */
#define CLONE_FS        0x00000200      /* set if fs info shared between processes */
#define CLONE_FILES     0x00000400      /* set if open files shared between processes */
#define CLONE_SIGHAND   0x00000800      /* set if signal handlers shared */
#define CLONE_IDLETASK  0x00001000      /* kernel-only flag */
#define CLONE_PTRACE    0x00002000      /* set if we want to let tracing continue on the child too */
#define CLONE_VFORK     0x00004000      /* set if the parent wants the child to wake it up on mm_release */
#define CLONE_PARENT    0x00008000      /* set if we want to have the same parent as the cloner */
#define CLONE_THREAD	0x00010000	/* Same thread group? */
#define CLONE_NEWNS	0x00020000	/* New namespace group? */
#define CLONE_SYSVSEM	0x00040000	/* share system V SEM_UNDO semantics */
#define CLONE_SETTLS	0x00080000	/* create a new TLS for the child */
#define CLONE_PARENT_SETTID	0x00100000	/* set the TID in the parent */
#define CLONE_CHILD_CLEARTID	0x00200000	/* clear the TID in the child */
#define CLONE_DETACHED		0x00400000	/* parent wants no child-exit signal */
#define CLONE_UNTRACED		0x00800000	/* set if the tracing process can't force CLONE_PTRACE on this clone */
#define CLONE_CHILD_SETTID	0x01000000	/* set the TID in the child */

static struct xlat clone_flags[] = {
    { CLONE_VM,		"CLONE_VM"	},
    { CLONE_FS,		"CLONE_FS"	},
    { CLONE_FILES,	"CLONE_FILES"	},
    { CLONE_SIGHAND,	"CLONE_SIGHAND"	},
    { CLONE_IDLETASK,	"CLONE_IDLETASK"},
    { CLONE_PTRACE,	"CLONE_PTRACE"	},
    { CLONE_VFORK,	"CLONE_VFORK"	},
    { CLONE_PARENT,	"CLONE_PARENT"	},
    { CLONE_THREAD,	"CLONE_THREAD" },
    { CLONE_NEWNS,	"CLONE_NEWNS" },
    { CLONE_SYSVSEM,	"CLONE_SYSVSEM" },
    { CLONE_SETTLS,	"CLONE_SETTLS" },
    { CLONE_PARENT_SETTID,"CLONE_PARENT_SETTID" },
    { CLONE_CHILD_CLEARTID,"CLONE_CHILD_CLEARTID" },
    { CLONE_DETACHED,	"CLONE_DETACHED" },
    { CLONE_UNTRACED,	"CLONE_UNTRACED" },
    { CLONE_CHILD_SETTID,"CLONE_CHILD_SETTID" },
    { 0,		NULL		},
};

# ifdef I386
#  include <asm/ldt.h>
extern void print_ldt_entry();
# endif

int
sys_clone(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		long flags, stack;
# if defined S390 || defined S390X
		/* For some reason, S390 has the stack argument first.  */
		stack = tcp->u_arg[0];
		flags = tcp->u_arg[1];
# else
		flags = tcp->u_arg[0];
		stack = tcp->u_arg[1];
# endif
		tprintf("child_stack=%#lx, flags=", stack);
		if (printflags(clone_flags, flags) == 0)
			tprintf("0");
		if ((flags & (CLONE_PARENT_SETTID|CLONE_CHILD_SETTID
				      |CLONE_SETTLS)) == 0)
			return 0;
		if (flags & CLONE_PARENT_SETTID) {
			int pid;
			if (umove(tcp, tcp->u_arg[2], &pid) == 0)
				tprintf(", [%d]", pid);
			else
				tprintf(", %#lx", tcp->u_arg[2]);
		}
		else
			tprintf(", <ignored>");
#ifdef I386
		if (flags & CLONE_SETTLS) {
			struct modify_ldt_ldt_s copy;
			if (umove(tcp, tcp->u_arg[3], &copy) != -1) {
				tprintf(", {entry_number:%d, ",
					copy.entry_number);
				if (!verbose(tcp))
					tprintf("...}");
				else
					print_ldt_entry(&copy);
			}
			else
				tprintf(", %#lx", tcp->u_arg[3]);
		}
		else
			tprintf(", <ignored>");
# define TIDARG 4
#else
# define TIDARG 3
#endif
		if (flags & CLONE_CHILD_SETTID)
			tprintf(", %#lx", tcp->u_arg[TIDARG]);
#undef TIDARG
	}
	return 0;
}

int
sys_clone2(tcp)
struct tcb *tcp;
{
       if (exiting(tcp)) {
               tprintf("child_stack=%#lx, stack_size=%#lx, flags=",
                       tcp->u_arg[1], tcp->u_arg[2]);
               if (printflags(clone_flags, tcp->u_arg[0]) == 0)
                       tprintf("0");
       }
       return 0;
}

#endif

int
sys_fork(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}

int
change_syscall(tcp, new)
struct tcb *tcp;
int new;
{
#if defined(LINUX)
#if defined(I386)
	/* Attempt to make vfork into fork, which we can follow. */
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(ORIG_EAX * 4), new) < 0)
		return -1;
	return 0;
#elif defined(X86_64)
	/* Attempt to make vfork into fork, which we can follow. */
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(ORIG_RAX * 8), new) < 0)
		return -1;
	return 0;
#elif defined(POWERPC)
	if (ptrace(PTRACE_POKEUSER, tcp->pid,
		   (char*)(sizeof(unsigned long)*PT_R0), new) < 0)
		return -1;
       return 0;
#elif defined(S390) || defined(S390X)
	/* s390 linux after 2.4.7 has a hook in entry.S to allow this */
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_GPR2), new)<0)
	        return -1;
	return 0;
#elif defined(M68K)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(4*PT_ORIG_D0), new)<0)
	    	return -1;
	return 0;
#elif defined(SPARC)
	struct regs regs;
	if (ptrace(PTRACE_GETREGS, tcp->pid, (char*)&regs, 0)<0)
		return -1;
	regs.r_g1=new;
	if (ptrace(PTRACE_SETREGS, tcp->pid, (char*)&regs, 0)<0)
	    	return -1;
	return 0;
#elif defined(MIPS)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_V0), new)<0)
	    	return -1;
	return 0;
#elif defined(ALPHA)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_A3), new)<0)
	    	return -1;
	return 0;
#elif defined(IA64)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_R15), new)<0)
		return -1;
	return 0;
#elif defined(HPPA)
	if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(PT_GR20), new)<0)
	    	return -1;
	return 0;
#elif defined(SH)
       if (ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(REG_SYSCALL), new)<0)
               return -1;
       return 0;
#else
#warning Do not know how to handle change_syscall for this architecture
#endif /* architecture */
#endif /* LINUX */
	return -1;
}

int
setarg(tcp, argnum)
	struct tcb *tcp;
	int argnum;
{
#if defined (IA64)
	{
		unsigned long *bsp, *ap;

		if (upeek(tcp->pid, PT_AR_BSP, (long *) &bsp) , 0)
			return -1;

		ap = ia64_rse_skip_regs(bsp, argnum);
		errno = 0;
		ptrace(PTRACE_POKEDATA, tcp->pid, (char *) ap, tcp->u_arg[argnum]);
		if (errno)
			return -1;

	}
#elif defined(I386)
	{
		ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(4*argnum), tcp->u_arg[argnum]);
		if (errno)
			return -1;
	}
#elif defined(X86_64)
	{
		ptrace(PTRACE_POKEUSER, tcp->pid, (char*)(8*(long)argnum), tcp->u_arg[argnum]);
		if (errno)
			return -1;
	}
#elif defined(POWERPC)
#ifndef PT_ORIG_R3
#define PT_ORIG_R3 34
#endif
	{
		ptrace(PTRACE_POKEUSER, tcp->pid,
		       (char*)((argnum==0 ? PT_ORIG_R3 : argnum+PT_R3)*sizeof(unsigned long)),
		       tcp->u_arg[argnum]);
		if (errno)
			return -1;
	}
#elif defined(MIPS)
	{
		errno = 0;
		if (argnum < 4)
			ptrace(PTRACE_POKEUSER, tcp->pid,
			       (char*)(REG_A0 + argnum), tcp->u_arg[argnum]);
		else {
			unsigned long *sp;

			if (upeek(tcp->pid, REG_SP, (long *) &sp) , 0)
				return -1;

			ptrace(PTRACE_POKEDATA, tcp->pid,
			       (char*)(sp + argnum - 4), tcp->u_arg[argnum]);
		}
		if (errno)
			return -1;
	}
#elif defined(S390) || defined(S390X)
        {
		if(argnum <= 5)
			ptrace(PTRACE_POKEUSER, tcp->pid,
			       (char *) (argnum==0 ? PT_ORIGGPR2 :
			       PT_GPR2 + argnum*sizeof(long)),
			       tcp->u_arg[argnum]);
		else
			return -E2BIG;
		if (errno)
			return -1;
        }
#else
# warning Sorry, setargs not implemented for this architecture.
#endif
	return 0;
}

#if defined SYS_clone || defined SYS_clone2
int
internal_clone(tcp)
struct tcb *tcp;
{
	struct tcb *tcpchild;
	int pid;
	if (entering(tcp)) {
		if (!followfork)
			return 0;
		if (fork_tcb(tcp))
			return 0;
		if (setbpt(tcp) < 0)
			return 0;
	} else {
		int bpt = tcp->flags & TCB_BPTSET;

		if (!(tcp->flags & TCB_FOLLOWFORK))
			return 0;

		if (syserror(tcp)) {
			if (bpt)
				clearbpt(tcp);
			return 0;
		}

		pid = tcp->u_rval;

#ifdef CLONE_PTRACE		/* See new setbpt code.  */
		tcpchild = pid2tcb(pid);
		if (tcpchild != NULL) {
			/* The child already reported its startup trap
			   before the parent reported its syscall return.  */
			if ((tcpchild->flags
			     & (TCB_STARTUP|TCB_ATTACHED|TCB_SUSPENDED))
			    != (TCB_STARTUP|TCB_ATTACHED|TCB_SUSPENDED))
				fprintf(stderr, "\
[preattached child %d of %d in weird state!]\n",
					pid, tcp->pid);
		}
		else
#endif
		if ((tcpchild = alloctcb(pid)) == NULL) {
			if (bpt)
				clearbpt(tcp);
			fprintf(stderr, " [tcb table full]\n");
			kill(pid, SIGKILL); /* XXX */
			return 0;
		}

#ifndef CLONE_PTRACE
		/* Attach to the new child */
		if (ptrace(PTRACE_ATTACH, pid, (char *) 1, 0) < 0) {
			if (bpt)
				clearbpt(tcp);
			perror("PTRACE_ATTACH");
			fprintf(stderr, "Too late?\n");
			droptcb(tcpchild);
			return 0;
		}
#endif

		if (bpt)
			clearbpt(tcp);

		tcpchild->flags |= TCB_ATTACHED;
		/* Child has BPT too, must be removed on first occasion.  */
		if (bpt) {
			tcpchild->flags |= TCB_BPTSET;
			tcpchild->baddr = tcp->baddr;
			memcpy(tcpchild->inst, tcp->inst,
				sizeof tcpchild->inst);
		}
		tcpchild->parent = tcp;
 		tcp->nchildren++;
		if (tcpchild->flags & TCB_SUSPENDED) {
			/* The child was born suspended, due to our having
			   forced CLONE_PTRACE.  */
			if (bpt)
				clearbpt(tcpchild);

			tcpchild->flags &= ~(TCB_SUSPENDED|TCB_STARTUP);
			if (ptrace(PTRACE_SYSCALL, pid, (char *) 1, 0) < 0) {
				perror("resume: ptrace(PTRACE_SYSCALL, ...)");
				return -1;
			}

			if (!qflag)
				fprintf(stderr, "\
Process %u resumed (parent %d ready)\n",
					pid, tcp->pid);
		}
		else {
			newoutf(tcpchild);
			if (!qflag)
				fprintf(stderr, "Process %d attached\n", pid);
		}

#ifdef TCB_CLONE_THREAD
		if ((tcp->flags & TCB_CLONE_THREAD) && tcp->parent != NULL) {
			/* The parent in this clone is itself a thread
			   belonging to another process.  There is no
			   meaning to the parentage relationship of the new
			   child with the thread, only with the process.
			   We associate the new thread with our parent.
			   Since this is done for every new thread, there
			   will never be a TCB_CLONE_THREAD process that
			   has children.  */
			--tcp->nchildren;
			tcp->u_arg[0] = tcp->parent->u_arg[0];
			tcp = tcp->parent;
			tcpchild->parent = tcp;
			++tcp->nchildren;
		}

		if (tcp->u_arg[0] & CLONE_THREAD) {
			tcpchild->flags |= TCB_CLONE_THREAD;
			++tcp->nclone_threads;
		}
		if (tcp->u_arg[0] & CLONE_DETACHED) {
			tcpchild->flags |= TCB_CLONE_DETACHED;
			++tcp->nclone_detached;
		}
#endif

 	}
	return 0;
}
#endif

int
internal_fork(tcp)
struct tcb *tcp;
{
#ifdef LINUX
	/* We do special magic with clone for any clone or fork.  */
	return internal_clone(tcp);
#else

	struct tcb *tcpchild;
	int pid;
	int dont_follow = 0;

#ifdef SYS_vfork
	if (tcp->scno == SYS_vfork) {
		/* Attempt to make vfork into fork, which we can follow. */
		if (!followvfork ||
		    change_syscall(tcp, SYS_fork) < 0)
			dont_follow = 1;
	}
#endif
	if (entering(tcp)) {
		if (!followfork || dont_follow)
			return 0;
		if (fork_tcb(tcp))
			return 0;
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
#ifdef HPPA
		/* The child must have run before it can be attached. */
		/* This must be a bug in the parisc kernel, but I havn't
		 * identified it yet.  Seems to be an issue associated
		 * with attaching to a process (which sends it a signal)
		 * before that process has ever been scheduled.  When
		 * debugging, I started seeing crashes in
		 * arch/parisc/kernel/signal.c:do_signal(), apparently
		 * caused by r8 getting corrupt over the dequeue_signal()
		 * call.  Didn't make much sense though...
		 */
		{
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 10000;
			select(0, NULL, NULL, NULL, &tv);
		}
#endif
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
#endif
}

#endif /* !USE_PROCFS */

#if defined(SUNOS4) || defined(LINUX) || defined(FREEBSD)

int
sys_vfork(tcp)
struct tcb *tcp;
{
	if (exiting(tcp))
		return RVAL_UDECIMAL;
	return 0;
}

#endif /* SUNOS4 || LINUX || FREEBSD */

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

#if defined(LINUX) || defined(FREEBSD)
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

#endif /* LINUX || FREEBSD */

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

#if UNIXWARE >= 2

#include <sys/privilege.h>


static struct xlat procpriv_cmds [] = {
	{ SETPRV,	"SETPRV"	},
	{ CLRPRV,	"CLRPRV"	},
	{ PUTPRV,	"PUTPRV"	},
	{ GETPRV,	"GETPRV"	},
	{ CNTPRV,	"CNTPRV"	},
	{ 0,		NULL		},
};


static struct xlat procpriv_priv [] = {
	{ P_OWNER,	"P_OWNER"	},
	{ P_AUDIT,	"P_AUDIT"	},
	{ P_COMPAT,	"P_COMPAT"	},
	{ P_DACREAD,	"P_DACREAD"	},
	{ P_DACWRITE,	"P_DACWRITE"	},
	{ P_DEV,	"P_DEV"		},
	{ P_FILESYS,	"P_FILESYS"	},
	{ P_MACREAD,	"P_MACREAD"	},
	{ P_MACWRITE,	"P_MACWRITE"	},
	{ P_MOUNT,	"P_MOUNT"	},
	{ P_MULTIDIR,	"P_MULTIDIR"	},
	{ P_SETPLEVEL,	"P_SETPLEVEL"	},
	{ P_SETSPRIV,	"P_SETSPRIV"	},
	{ P_SETUID,	"P_SETUID"	},
	{ P_SYSOPS,	"P_SYSOPS"	},
	{ P_SETUPRIV,	"P_SETUPRIV"	},
	{ P_DRIVER,	"P_DRIVER"	},
	{ P_RTIME,	"P_RTIME"	},
	{ P_MACUPGRADE,	"P_MACUPGRADE"	},
	{ P_FSYSRANGE,	"P_FSYSRANGE"	},
	{ P_SETFLEVEL,	"P_SETFLEVEL"	},
	{ P_AUDITWR,	"P_AUDITWR"	},
	{ P_TSHAR,	"P_TSHAR"	},
	{ P_PLOCK,	"P_PLOCK"	},
	{ P_CORE,	"P_CORE"	},
	{ P_LOADMOD,	"P_LOADMOD"	},
	{ P_BIND,	"P_BIND"	},
	{ P_ALLPRIVS,	"P_ALLPRIVS"	},
	{ 0,		NULL		},
};


static struct xlat procpriv_type [] = {
	{ PS_FIX,	"PS_FIX"	},
	{ PS_INH,	"PS_INH"	},
	{ PS_MAX,	"PS_MAX"	},
	{ PS_WKG,	"PS_WKG"	},
	{ 0,		NULL		},
};


static void
printpriv(tcp, addr, len, opt)
struct tcb *tcp;
long addr;
int len;
struct xlat *opt;
{
	priv_t buf [128];
	int max = verbose (tcp) ? sizeof buf / sizeof buf [0] : 10;
	int dots = len > max;
	int i;

	if (len > max) len = max;

	if (len <= 0 ||
	    umoven (tcp, addr, len * sizeof buf[0], (char *) buf) < 0)
	{
		tprintf ("%#lx", addr);
		return;
	}

	tprintf ("[");

	for (i = 0; i < len; ++i) {
		char *t, *p;

		if (i) tprintf (", ");

		if ((t = xlookup (procpriv_type, buf [i] & PS_TYPE)) &&
		    (p = xlookup (procpriv_priv, buf [i] & ~PS_TYPE)))
		{
			tprintf ("%s|%s", t, p);
		}
		else {
			tprintf ("%#lx", buf [i]);
		}
	}

	if (dots) tprintf (" ...");

	tprintf ("]");
}


int
sys_procpriv(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printxval(procpriv_cmds, tcp->u_arg[0], "???PRV");
		switch (tcp->u_arg[0]) {
		    case CNTPRV:
			tprintf(", %#lx, %ld", tcp->u_arg[1], tcp->u_arg[2]);
			break;

		    case GETPRV:
			break;

		    default:
			tprintf (", ");
			printpriv (tcp, tcp->u_arg[1], tcp->u_arg[2]);
			tprintf (", %ld", tcp->u_arg[2]);
		}
	}
	else if (tcp->u_arg[0] == GETPRV) {
		if (syserror (tcp)) {
			tprintf(", %#lx, %ld", tcp->u_arg[1], tcp->u_arg[2]);
		}
		else {
			tprintf (", ");
			printpriv (tcp, tcp->u_arg[1], tcp->u_rval);
			tprintf (", %ld", tcp->u_arg[2]);
		}
	}

	return 0;
}

#endif


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
#if defined LINUX && defined TCB_WAITEXECVE
	tcp->flags |= TCB_WAITEXECVE;
#endif /* LINUX && TCB_WAITEXECVE */
	return 0;
}

#if UNIXWARE > 2

int sys_rexecve(tcp)
struct tcb *tcp;
{
	if (entering (tcp)) {
		sys_execve (tcp);
		tprintf (", %ld", tcp->u_arg[3]);
	}
	return 0;
}

#endif

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
#ifndef __WNOTHREAD
#define __WNOTHREAD	0x20000000
#endif
#ifndef __WALL
#define __WALL		0x40000000
#endif
#ifndef __WCLONE
#define __WCLONE	0x80000000
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
#ifdef __WALL
	{ __WALL,	"__WALL"	},
#endif
#ifdef __WNOTHREAD
	{ __WNOTHREAD,	"__WNOTHREAD"	},
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
printwaitn(tcp, n, bitness)
struct tcb *tcp;
int n;
int bitness;
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
			else if (tcp->u_rval > 0) {
#ifdef LINUX_64BIT
				if (bitness)
					printrusage32(tcp, tcp->u_arg[3]);
				else
#endif
					printrusage(tcp, tcp->u_arg[3]);
			}
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
	int got_kids;

#ifdef TCB_CLONE_THREAD
	if (tcp->flags & TCB_CLONE_THREAD)
		/* The children we wait for are our parent's children.  */
		got_kids = (tcp->parent->nchildren
			    > tcp->parent->nclone_detached);
	else
		got_kids = (tcp->nchildren > tcp->nclone_detached);
#else
	got_kids = tcp->nchildren > 0;
#endif

	if (entering(tcp) && got_kids) {
		/* There are children that this parent should block for.
		   But ptrace made us the parent of the traced children
		   and the real parent will get ECHILD from the wait call.

		   XXX If we attached with strace -f -p PID, then there
		   may be untraced dead children the parent could be reaping
		   now, but we make him block.  */

		/* ??? WTA: fix bug with hanging children */

		if (!(tcp->u_arg[2] & WNOHANG)) {
			/* There are traced children */
			tcp->flags |= TCB_SUSPENDED;
			tcp->waitpid = tcp->u_arg[0];
#ifdef TCB_CLONE_THREAD
			if (tcp->flags & TCB_CLONE_THREAD)
				tcp->parent->nclone_waiting++;
#endif
		}
	}
	if (exiting(tcp) && tcp->u_error == ECHILD && got_kids) {
		if (tcp->u_arg[2] & WNOHANG) {
			/* We must force a fake result of 0 instead of
			   the ECHILD error.  */
			extern int force_result();
			return force_result(tcp, 0, 0);
		}
		else
			fprintf(stderr,
				"internal_wait: should not have resumed %d\n",
				tcp->pid);
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

#ifdef FREEBSD
int
sys_wait(tcp)
struct tcb *tcp;
{
	int status;

	if (exiting(tcp)) {
		if (!syserror(tcp)) {
			if (umove(tcp, tcp->u_arg[0], &status) < 0)
				tprintf("%#lx", tcp->u_arg[0]);
			else
				printstatus(status);
		}
	}
	return 0;
}
#endif

int
sys_waitpid(tcp)
struct tcb *tcp;
{
	return printwaitn(tcp, 3, 0);
}

int
sys_wait4(tcp)
struct tcb *tcp;
{
	return printwaitn(tcp, 4, 0);
}

#ifdef ALPHA
int
sys_osf_wait4(tcp)
struct tcb *tcp;
{
	return printwaitn(tcp, 4, 1);
}
#endif

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
			printsiginfo(&si, verbose (tcp));
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
#ifndef FREEBSD
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
#ifdef PTRACE_GETREGS
	{ PTRACE_GETREGS,	"PTRACE_GETREGS"	},
#endif
#ifdef PTRACE_SETREGS
	{ PTRACE_SETREGS,	"PTRACE_SETREGS"	},
#endif
#ifdef PTRACE_GETFPREGS
	{ PTRACE_GETFPREGS,	"PTRACE_GETFPREGS",	},
#endif
#ifdef PTRACE_SETFPREGS
	{ PTRACE_SETFPREGS,	"PTRACE_SETFPREGS",	},
#endif
#ifdef PTRACE_GETFPXREGS
	{ PTRACE_GETFPXREGS,	"PTRACE_GETFPXREGS",	},
#endif
#ifdef PTRACE_SETFPXREGS
	{ PTRACE_SETFPXREGS,	"PTRACE_SETFPXREGS",	},
#endif
#ifdef SUNOS4
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
#else /* FREEBSD */
	{ PT_TRACE_ME,		"PT_TRACE_ME"		},
	{ PT_READ_I,		"PT_READ_I"		},
	{ PT_READ_D,		"PT_READ_D"		},
	{ PT_WRITE_I,		"PT_WRITE_I"		},
	{ PT_WRITE_D,		"PT_WRITE_D"		},
#ifdef PT_READ_U
	{ PT_READ_U,		"PT_READ_U"		},
#endif
	{ PT_CONTINUE,		"PT_CONTINUE"		},
	{ PT_KILL,		"PT_KILL"		},
	{ PT_STEP,		"PT_STEP"		},
	{ PT_ATTACH,		"PT_ATTACH"		},
	{ PT_DETACH,		"PT_DETACH"		},
	{ PT_GETREGS,		"PT_GETREGS"		},
	{ PT_SETREGS,		"PT_SETREGS"		},
	{ PT_GETFPREGS,		"PT_GETFPREGS"		},
	{ PT_SETFPREGS,		"PT_SETFPREGS"		},
	{ PT_GETDBREGS,		"PT_GETDBREGS"		},
	{ PT_SETDBREGS,		"PT_SETDBREGS"		},
#endif /* FREEBSD */
	{ 0,			NULL			},
};

#ifndef FREEBSD
#ifndef SUNOS4_KERNEL_ARCH_KLUDGE
static
#endif /* !SUNOS4_KERNEL_ARCH_KLUDGE */
struct xlat struct_user_offsets[] = {
#ifdef LINUX
#if defined(S390) || defined(S390X)
	{ PT_PSWMASK,		"psw_mask"				},
	{ PT_PSWADDR,		"psw_addr"				},
	{ PT_GPR0,		"gpr0"					},
	{ PT_GPR1,		"gpr1"					},
	{ PT_GPR2,		"gpr2"					},
	{ PT_GPR3,		"gpr3"					},
	{ PT_GPR4,		"gpr4"					},
	{ PT_GPR5,		"gpr5"					},
	{ PT_GPR6,		"gpr6"					},
	{ PT_GPR7,		"gpr7"					},
	{ PT_GPR8,		"gpr8"					},
	{ PT_GPR9,		"gpr9"					},
	{ PT_GPR10,		"gpr10"					},
	{ PT_GPR11,		"gpr11"					},
	{ PT_GPR12,		"gpr12"					},
	{ PT_GPR13,		"gpr13"					},
	{ PT_GPR14,		"gpr14"					},
	{ PT_GPR15,		"gpr15"					},
	{ PT_ACR0,		"acr0"					},
	{ PT_ACR1,		"acr1"					},
	{ PT_ACR2,		"acr2"					},
	{ PT_ACR3,		"acr3"					},
	{ PT_ACR4,		"acr4"					},
	{ PT_ACR5,		"acr5"					},
	{ PT_ACR6,		"acr6"					},
	{ PT_ACR7,		"acr7"					},
	{ PT_ACR8,		"acr8"					},
	{ PT_ACR9,		"acr9"					},
	{ PT_ACR10,		"acr10"					},
	{ PT_ACR11,		"acr11"					},
	{ PT_ACR12,		"acr12"					},
	{ PT_ACR13,		"acr13"					},
	{ PT_ACR14,		"acr14"					},
	{ PT_ACR15,		"acr15"					},
	{ PT_ORIGGPR2,		"orig_gpr2"				},
	{ PT_FPC,		"fpc"					},
#if defined(S390)
	{ PT_FPR0_HI,		"fpr0.hi"				},
	{ PT_FPR0_LO,		"fpr0.lo"				},
	{ PT_FPR1_HI,		"fpr1.hi"				},
	{ PT_FPR1_LO,		"fpr1.lo"				},
	{ PT_FPR2_HI,		"fpr2.hi"				},
	{ PT_FPR2_LO,		"fpr2.lo"				},
	{ PT_FPR3_HI,		"fpr3.hi"				},
	{ PT_FPR3_LO,		"fpr3.lo"				},
	{ PT_FPR4_HI,		"fpr4.hi"				},
	{ PT_FPR4_LO,		"fpr4.lo"				},
	{ PT_FPR5_HI,		"fpr5.hi"				},
	{ PT_FPR5_LO,		"fpr5.lo"				},
	{ PT_FPR6_HI,		"fpr6.hi"				},
	{ PT_FPR6_LO,		"fpr6.lo"				},
	{ PT_FPR7_HI,		"fpr7.hi"				},
	{ PT_FPR7_LO,		"fpr7.lo"				},
	{ PT_FPR8_HI,		"fpr8.hi"				},
	{ PT_FPR8_LO,		"fpr8.lo"				},
	{ PT_FPR9_HI,		"fpr9.hi"				},
	{ PT_FPR9_LO,		"fpr9.lo"				},
	{ PT_FPR10_HI,		"fpr10.hi"				},
	{ PT_FPR10_LO,		"fpr10.lo"				},
	{ PT_FPR11_HI,		"fpr11.hi"				},
	{ PT_FPR11_LO,		"fpr11.lo"				},
	{ PT_FPR12_HI,		"fpr12.hi"				},
	{ PT_FPR12_LO,		"fpr12.lo"				},
	{ PT_FPR13_HI,		"fpr13.hi"				},
	{ PT_FPR13_LO,		"fpr13.lo"				},
	{ PT_FPR14_HI,		"fpr14.hi"				},
	{ PT_FPR14_LO,		"fpr14.lo"				},
	{ PT_FPR15_HI,		"fpr15.hi"				},
	{ PT_FPR15_LO,		"fpr15.lo"				},
#endif
#if defined(S390X)
	{ PT_FPR0,		"fpr0"					},
	{ PT_FPR1,		"fpr1"					},
	{ PT_FPR2,		"fpr2"					},
	{ PT_FPR3,		"fpr3"					},
	{ PT_FPR4,		"fpr4"					},
	{ PT_FPR5,		"fpr5"					},
	{ PT_FPR6,		"fpr6"					},
	{ PT_FPR7,		"fpr7"					},
	{ PT_FPR8,		"fpr8"					},
	{ PT_FPR9,		"fpr9"					},
	{ PT_FPR10,		"fpr10"					},
	{ PT_FPR11,		"fpr11"					},
	{ PT_FPR12,		"fpr12"					},
	{ PT_FPR13,		"fpr13"					},
	{ PT_FPR14,		"fpr14"					},
	{ PT_FPR15,		"fpr15"					},
#endif
	{ PT_CR_9,		"cr9"					},
	{ PT_CR_10,		"cr10"					},
	{ PT_CR_11,		"cr11"					},
	{ PT_IEEE_IP,           "ieee_exception_ip"                     },
#endif
#if defined(SPARC)
	/* XXX No support for these offsets yet. */
#elif defined(HPPA)
	/* XXX No support for these offsets yet. */
#elif defined(POWERPC)
#ifndef PT_ORIG_R3
#define PT_ORIG_R3 34
#endif
#define REGSIZE (sizeof(unsigned long))
	{ REGSIZE*PT_R0,		"r0"				},
	{ REGSIZE*PT_R1,		"r1"				},
	{ REGSIZE*PT_R2,		"r2"				},
	{ REGSIZE*PT_R3,		"r3"				},
	{ REGSIZE*PT_R4,		"r4"				},
	{ REGSIZE*PT_R5,		"r5"				},
	{ REGSIZE*PT_R6,		"r6"				},
	{ REGSIZE*PT_R7,		"r7"				},
	{ REGSIZE*PT_R8,		"r8"				},
	{ REGSIZE*PT_R9,		"r9"				},
	{ REGSIZE*PT_R10,		"r10"				},
	{ REGSIZE*PT_R11,		"r11"				},
	{ REGSIZE*PT_R12,		"r12"				},
	{ REGSIZE*PT_R13,		"r13"				},
	{ REGSIZE*PT_R14,		"r14"				},
	{ REGSIZE*PT_R15,		"r15"				},
	{ REGSIZE*PT_R16,		"r16"				},
	{ REGSIZE*PT_R17,		"r17"				},
	{ REGSIZE*PT_R18,		"r18"				},
	{ REGSIZE*PT_R19,		"r19"				},
	{ REGSIZE*PT_R20,		"r20"				},
	{ REGSIZE*PT_R21,		"r21"				},
	{ REGSIZE*PT_R22,		"r22"				},
	{ REGSIZE*PT_R23,		"r23"				},
	{ REGSIZE*PT_R24,		"r24"				},
	{ REGSIZE*PT_R25,		"r25"				},
	{ REGSIZE*PT_R26,		"r26"				},
	{ REGSIZE*PT_R27,		"r27"				},
	{ REGSIZE*PT_R28,		"r28"				},
	{ REGSIZE*PT_R29,		"r29"				},
	{ REGSIZE*PT_R30,		"r30"				},
	{ REGSIZE*PT_R31,		"r31"				},
	{ REGSIZE*PT_NIP,		"NIP"				},
	{ REGSIZE*PT_MSR,		"MSR"				},
	{ REGSIZE*PT_ORIG_R3,		"ORIG_R3"			},
	{ REGSIZE*PT_CTR,		"CTR"				},
	{ REGSIZE*PT_LNK,		"LNK"				},
	{ REGSIZE*PT_XER,		"XER"				},
	{ REGSIZE*PT_CCR,		"CCR"				},
	{ REGSIZE*PT_FPR0,		"FPR0"				},
#undef REGSIZE
#else
#ifdef ALPHA
	{ 0,			"r0"					},
	{ 1,			"r1"					},
	{ 2,			"r2"					},
	{ 3,			"r3"					},
	{ 4,			"r4"					},
	{ 5,			"r5"					},
	{ 6,			"r6"					},
	{ 7,			"r7"					},
	{ 8,			"r8"					},
	{ 9,			"r9"					},
	{ 10,			"r10"					},
	{ 11,			"r11"					},
	{ 12,			"r12"					},
	{ 13,			"r13"					},
	{ 14,			"r14"					},
	{ 15,			"r15"					},
	{ 16,			"r16"					},
	{ 17,			"r17"					},
	{ 18,			"r18"					},
	{ 19,			"r19"					},
	{ 20,			"r20"					},
	{ 21,			"r21"					},
	{ 22,			"r22"					},
	{ 23,			"r23"					},
	{ 24,			"r24"					},
	{ 25,			"r25"					},
	{ 26,			"r26"					},
	{ 27,			"r27"					},
	{ 28,			"r28"					},
	{ 29,			"gp"					},
	{ 30,			"fp"					},
	{ 31,			"zero"					},
	{ 32,			"fp0"					},
	{ 33,			"fp"					},
	{ 34,			"fp2"					},
	{ 35,			"fp3"					},
	{ 36,			"fp4"					},
	{ 37,			"fp5"					},
	{ 38,			"fp6"					},
	{ 39,			"fp7"					},
	{ 40,			"fp8"					},
	{ 41,			"fp9"					},
	{ 42,			"fp10"					},
	{ 43,			"fp11"					},
	{ 44,			"fp12"					},
	{ 45,			"fp13"					},
	{ 46,			"fp14"					},
	{ 47,			"fp15"					},
	{ 48,			"fp16"					},
	{ 49,			"fp17"					},
	{ 50,			"fp18"					},
	{ 51,			"fp19"					},
	{ 52,			"fp20"					},
	{ 53,			"fp21"					},
	{ 54,			"fp22"					},
	{ 55,			"fp23"					},
	{ 56,			"fp24"					},
	{ 57,			"fp25"					},
	{ 58,			"fp26"					},
	{ 59,			"fp27"					},
	{ 60,			"fp28"					},
	{ 61,			"fp29"					},
	{ 62,			"fp30"					},
	{ 63,			"fp31"					},
	{ 64,			"pc"					},
#else /* !ALPHA */
#ifdef IA64
	{ PT_F32, "f32" }, { PT_F33, "f33" }, { PT_F34, "f34" },
	{ PT_F35, "f35" }, { PT_F36, "f36" }, { PT_F37, "f37" },
	{ PT_F38, "f38" }, { PT_F39, "f39" }, { PT_F40, "f40" },
	{ PT_F41, "f41" }, { PT_F42, "f42" }, { PT_F43, "f43" },
	{ PT_F44, "f44" }, { PT_F45, "f45" }, { PT_F46, "f46" },
	{ PT_F47, "f47" }, { PT_F48, "f48" }, { PT_F49, "f49" },
	{ PT_F50, "f50" }, { PT_F51, "f51" }, { PT_F52, "f52" },
	{ PT_F53, "f53" }, { PT_F54, "f54" }, { PT_F55, "f55" },
	{ PT_F56, "f56" }, { PT_F57, "f57" }, { PT_F58, "f58" },
	{ PT_F59, "f59" }, { PT_F60, "f60" }, { PT_F61, "f61" },
	{ PT_F62, "f62" }, { PT_F63, "f63" }, { PT_F64, "f64" },
	{ PT_F65, "f65" }, { PT_F66, "f66" }, { PT_F67, "f67" },
	{ PT_F68, "f68" }, { PT_F69, "f69" }, { PT_F70, "f70" },
	{ PT_F71, "f71" }, { PT_F72, "f72" }, { PT_F73, "f73" },
	{ PT_F74, "f74" }, { PT_F75, "f75" }, { PT_F76, "f76" },
	{ PT_F77, "f77" }, { PT_F78, "f78" }, { PT_F79, "f79" },
	{ PT_F80, "f80" }, { PT_F81, "f81" }, { PT_F82, "f82" },
	{ PT_F83, "f83" }, { PT_F84, "f84" }, { PT_F85, "f85" },
	{ PT_F86, "f86" }, { PT_F87, "f87" }, { PT_F88, "f88" },
	{ PT_F89, "f89" }, { PT_F90, "f90" }, { PT_F91, "f91" },
	{ PT_F92, "f92" }, { PT_F93, "f93" }, { PT_F94, "f94" },
	{ PT_F95, "f95" }, { PT_F96, "f96" }, { PT_F97, "f97" },
	{ PT_F98, "f98" }, { PT_F99, "f99" }, { PT_F100, "f100" },
	{ PT_F101, "f101" }, { PT_F102, "f102" }, { PT_F103, "f103" },
	{ PT_F104, "f104" }, { PT_F105, "f105" }, { PT_F106, "f106" },
	{ PT_F107, "f107" }, { PT_F108, "f108" }, { PT_F109, "f109" },
	{ PT_F110, "f110" }, { PT_F111, "f111" }, { PT_F112, "f112" },
	{ PT_F113, "f113" }, { PT_F114, "f114" }, { PT_F115, "f115" },
	{ PT_F116, "f116" }, { PT_F117, "f117" }, { PT_F118, "f118" },
	{ PT_F119, "f119" }, { PT_F120, "f120" }, { PT_F121, "f121" },
	{ PT_F122, "f122" }, { PT_F123, "f123" }, { PT_F124, "f124" },
	{ PT_F125, "f125" }, { PT_F126, "f126" }, { PT_F127, "f127" },
	/* switch stack: */
	{ PT_F2, "f2" }, { PT_F3, "f3" }, { PT_F4, "f4" },
	{ PT_F5, "f5" }, { PT_F10, "f10" }, { PT_F11, "f11" },
	{ PT_F12, "f12" }, { PT_F13, "f13" }, { PT_F14, "f14" },
	{ PT_F15, "f15" }, { PT_F16, "f16" }, { PT_F17, "f17" },
	{ PT_F18, "f18" }, { PT_F19, "f19" }, { PT_F20, "f20" },
	{ PT_F21, "f21" }, { PT_F22, "f22" }, { PT_F23, "f23" },
	{ PT_F24, "f24" }, { PT_F25, "f25" }, { PT_F26, "f26" },
	{ PT_F27, "f27" }, { PT_F28, "f28" }, { PT_F29, "f29" },
	{ PT_F30, "f30" }, { PT_F31, "f31" }, { PT_R4, "r4" },
	{ PT_R5, "r5" }, { PT_R6, "r6" }, { PT_R7, "r7" },
	{ PT_B0, "kb0" },
	{ PT_B1, "b1" }, { PT_B2, "b2" }, { PT_B3, "b3" },
	{ PT_B4, "b4" }, { PT_B5, "b5" },
	{ PT_AR_PFS, "kar.pfs" },
	{ PT_AR_LC, "ar.lc" }, { PT_AR_UNAT, "kar.unat" },
	{ PT_AR_RNAT, "kar.rnat" }, { PT_AR_BSPSTORE, "kar.bspstore" },
	{ PT_PR, "k.pr" },
	/* pt_regs */
	{ PT_CR_IPSR, "cr.ipsr" }, { PT_CR_IIP, "cr.iip" },
	/*{ PT_CR_IFS, "cr.ifs" },*/ { PT_AR_UNAT, "ar.unat" },
	{ PT_AR_PFS, "ar.pfs" }, { PT_AR_RSC, "ar.rsc" },
	{ PT_AR_RNAT, "ar.rnat" }, { PT_AR_BSPSTORE, "ar.bspstore" },
	{ PT_PR, "pr" }, { PT_B6, "b6" }, { PT_AR_BSP, "ar.bsp" },
	{ PT_R1, "r1" }, { PT_R2, "r2" }, { PT_R3, "r3" },
	{ PT_R12, "r12" }, { PT_R13, "r13" }, { PT_R14, "r14" },
	{ PT_R15, "r15" }, { PT_R8, "r8" }, { PT_R9, "r9" },
	{ PT_R10, "r10" }, { PT_R11, "r11" }, { PT_R16, "r16" },
	{ PT_R17, "r17" }, { PT_R18, "r18" }, { PT_R19, "r19" },
	{ PT_R20, "r20" }, { PT_R21, "r21" }, { PT_R22, "r22" },
	{ PT_R23, "r23" }, { PT_R24, "r24" }, { PT_R25, "r25" },
	{ PT_R26, "r26" }, { PT_R27, "r27" }, { PT_R28, "r28" },
	{ PT_R29, "r29" }, { PT_R30, "r30" }, { PT_R31, "r31" },
	{ PT_AR_CCV, "ar.ccv" }, { PT_AR_FPSR, "ar.fpsr" },
	{ PT_B0, "b0" }, { PT_B7, "b7" }, { PT_F6, "f6" },
	{ PT_F7, "f7" }, { PT_F8, "f8" }, { PT_F9, "f9" },
#else /* !IA64 */
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
#ifdef X86_64
	{ 8*RDI,		"8*RDI"					},
	{ 8*RSI,		"8*RSI"					},
	{ 8*RDX,		"8*RDX"					},
	{ 8*R10, 		"8*R10" },
	{ 8*R8, 		"8*R8" },
	{ 8*R9, 		"8*R9" },
	{ 8*RBX,		"8*RBX"					},
	{ 8*RCX,		"8*RCX"					},
	{ 8*RBP,		"8*RBP"					},
	{ 8*RAX,		"8*RAX"					},
#if 0
	{ 8*DS,			"8*DS"					},
	{ 8*ES,			"8*ES"					},
	{ 8*FS,			"8*FS"					},
	{ 8*GS,			"8*GS"					},
#endif
	{ 8*ORIG_RAX,		"8*ORIG_EAX"				},
	{ 8*RIP,		"8*RIP"					},
	{ 8*CS,			"8*CS"					},
	{ 8*EFLAGS,		"8*EFL"					},
	{ 8*RSP,		"8*RSP"				},
	{ 8*SS,			"8*SS"					},
	{ 8*R11, 		"8*R11" },
	{ 8*R12, 		"8*R12" },
	{ 8*R13, 		"8*R13" },
	{ 8*R14, 		"8*R14" },
	{ 8*R15, 		"8*R15" },
#endif
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
#ifdef SH
       { 4*REG_REG0,           "4*REG_REG0"                            },
       { 4*(REG_REG0+1),       "4*REG_REG1"                            },
       { 4*(REG_REG0+2),       "4*REG_REG2"                            },
       { 4*(REG_REG0+3),       "4*REG_REG3"                            },
       { 4*(REG_REG0+4),       "4*REG_REG4"                            },
       { 4*(REG_REG0+5),       "4*REG_REG5"                            },
       { 4*(REG_REG0+6),       "4*REG_REG6"                            },
       { 4*(REG_REG0+7),       "4*REG_REG7"                            },
       { 4*(REG_REG0+8),       "4*REG_REG8"                            },
       { 4*(REG_REG0+9),       "4*REG_REG9"                            },
       { 4*(REG_REG0+10),      "4*REG_REG10"                           },
       { 4*(REG_REG0+11),      "4*REG_REG11"                           },
       { 4*(REG_REG0+12),      "4*REG_REG12"                           },
       { 4*(REG_REG0+13),      "4*REG_REG13"                           },
       { 4*(REG_REG0+14),      "4*REG_REG14"                           },
       { 4*REG_REG15,          "4*REG_REG15"                           },
       { 4*REG_PC,             "4*REG_PC"                              },
       { 4*REG_PR,             "4*REG_PR"                              },
       { 4*REG_SR,             "4*REG_SR"                              },
       { 4*REG_GBR,            "4*REG_GBR"                             },
       { 4*REG_MACH,           "4*REG_MACH"                            },
       { 4*REG_MACL,           "4*REG_MACL"                            },
       { 4*REG_SYSCALL,        "4*REG_SYSCALL"                         },
       { 4*REG_FPUL,           "4*REG_FPUL"                            },
       { 4*REG_FPREG0,         "4*REG_FPREG0"                          },
       { 4*(REG_FPREG0+1),     "4*REG_FPREG1"                          },
       { 4*(REG_FPREG0+2),     "4*REG_FPREG2"                          },
       { 4*(REG_FPREG0+3),     "4*REG_FPREG3"                          },
       { 4*(REG_FPREG0+4),     "4*REG_FPREG4"                          },
       { 4*(REG_FPREG0+5),     "4*REG_FPREG5"                          },
       { 4*(REG_FPREG0+6),     "4*REG_FPREG6"                          },
       { 4*(REG_FPREG0+7),     "4*REG_FPREG7"                          },
       { 4*(REG_FPREG0+8),     "4*REG_FPREG8"                          },
       { 4*(REG_FPREG0+9),     "4*REG_FPREG9"                          },
       { 4*(REG_FPREG0+10),    "4*REG_FPREG10"                         },
       { 4*(REG_FPREG0+11),    "4*REG_FPREG11"                         },
       { 4*(REG_FPREG0+12),    "4*REG_FPREG12"                         },
       { 4*(REG_FPREG0+13),    "4*REG_FPREG13"                         },
       { 4*(REG_FPREG0+14),    "4*REG_FPREG14"                         },
       { 4*REG_FPREG15,        "4*REG_FPREG15"                         },
       { 4*REG_XDREG0,         "4*REG_XDREG0"                          },
       { 4*(REG_XDREG0+2),     "4*REG_XDREG2"                          },
       { 4*(REG_XDREG0+4),     "4*REG_XDREG4"                          },
       { 4*(REG_XDREG0+6),     "4*REG_XDREG6"                          },
       { 4*(REG_XDREG0+8),     "4*REG_XDREG8"                          },
       { 4*(REG_XDREG0+10),    "4*REG_XDREG10"                         },
       { 4*(REG_XDREG0+12),    "4*REG_XDREG12"                         },
       { 4*REG_XDREG14,        "4*REG_XDREG14"                         },
       { 4*REG_FPSCR,          "4*REG_FPSCR"                           },
#endif /* SH */

#if !defined(S390) && !defined(S390X) && !defined(MIPS)
	{ uoff(u_fpvalid),	"offsetof(struct user, u_fpvalid)"	},
#endif
#if  defined(I386) || defined(X86_64)
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
#if !defined(S390) && !defined(S390X) && !defined(MIPS) && !defined(SH)
	{ uoff(reserved),	"offsetof(struct user, reserved)"	},
#endif
	{ uoff(u_ar0),		"offsetof(struct user, u_ar0)"		},
#if !defined(ARM) && !defined(MIPS) && !defined(S390) && !defined(S390X)
	{ uoff(u_fpstate),	"offsetof(struct user, u_fpstate)"	},
#endif
	{ uoff(magic),		"offsetof(struct user, magic)"		},
	{ uoff(u_comm),		"offsetof(struct user, u_comm)"		},
#if defined(I386) || defined(X86_64)
	{ uoff(u_debugreg),	"offsetof(struct user, u_debugreg)"	},
#endif /* I386 */
#endif /* !IA64 */
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
#ifndef HPPA
	{ sizeof(struct user),	"sizeof(struct user)"			},
#endif
	{ 0,			NULL					},
};
#endif

int
sys_ptrace(tcp)
struct tcb *tcp;
{
	struct xlat *x;
	long addr;

	if (entering(tcp)) {
		printxval(ptrace_cmds, tcp->u_arg[0],
#ifndef FREEBSD
			  "PTRACE_???"
#else
			  "PT_???"
#endif
			);
		tprintf(", %lu, ", tcp->u_arg[1]);
		addr = tcp->u_arg[2];
#ifndef FREEBSD
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
#endif
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
			printnum(tcp, tcp->u_arg[3], "%#lx");
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
#ifdef FREEBSD
		tprintf("%lu", tcp->u_arg[3]);
	}
#endif /* FREEBSD */
	return 0;
}

#endif /* !SVR4 */

#ifdef LINUX
static struct xlat futexops[] = {
	{ FUTEX_WAIT,	"FUTEX_WAIT" },
	{ FUTEX_WAKE,	"FUTEX_WAKE" },
	{ FUTEX_FD,	"FUTEX_FD" },
	{ 0,		NULL }
};

int
sys_futex(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	tprintf("%p, ", (void *) tcp->u_arg[0]);
	printflags(futexops, tcp->u_arg[1]);
	tprintf(", %ld, ", tcp->u_arg[2]);
	printtv(tcp, tcp->u_arg[3]);
    }
    return 0;
}

static void
print_affinitylist(list, len)
unsigned long *list;
unsigned int len;
{
    int first = 1;
    tprintf(" {");
    while (len > sizeof (unsigned long)) {
	tprintf("%s %lx", first ? "" : ",", *list++);
	first = 0;
	len -= sizeof (unsigned long);
    }
    tprintf(" }");
}

int
sys_sched_setaffinity(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
	print_affinitylist((unsigned long *) tcp->u_arg[2], tcp->u_arg[1]);
    }
    return 0;
}

int
sys_sched_getaffinity(tcp)
struct tcb *tcp;
{
    if (entering(tcp)) {
	tprintf("%ld, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
    } else {
	print_affinitylist((unsigned long *) tcp->u_arg[2], tcp->u_rval);
    }
    return 0;
}
#endif
