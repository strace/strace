/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
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

#include <signal.h>
#include <sys/user.h>
#include <fcntl.h>

#ifdef SVR4
#include <sys/ucontext.h>
#endif /* SVR4 */

#if HAVE_LINUX_PTRACE_H
#undef PTRACE_SYSCALL
#include <linux/ptrace.h>
#endif 

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
#ifndef PTRACE_PEEKUSR
# define PTRACE_PEEKUSR PTRACE_PEEKUSER
#endif
#ifndef PTRACE_POKEUSR
# define PTRACE_POKEUSR PTRACE_POKEUSER
#endif
#endif

#ifdef LINUX

#ifdef IA64
# include <asm/ptrace_offsets.h>
#endif /* !IA64 */

#ifdef HAVE_ASM_SIGCONTEXT_H
#include <asm/sigcontext.h>
#ifdef SPARC
#include <asm/reg.h>
typedef struct {
	struct regs		si_regs;
	int			si_mask;
} m_siginfo_t;
#endif
#else /* !HAVE_ASM_SIGCONTEXT_H */
#ifdef I386
struct sigcontext_struct {
	unsigned short gs, __gsh;
	unsigned short fs, __fsh;
	unsigned short es, __esh;
	unsigned short ds, __dsh;
	unsigned long edi;
	unsigned long esi;
	unsigned long ebp;
	unsigned long esp;
	unsigned long ebx;
	unsigned long edx;
	unsigned long ecx;
	unsigned long eax;
	unsigned long trapno;
	unsigned long err;
	unsigned long eip;
	unsigned short cs, __csh;
	unsigned long eflags;
	unsigned long esp_at_signal;
	unsigned short ss, __ssh;
	unsigned long i387;
	unsigned long oldmask;
	unsigned long cr2;
};
#else /* !I386 */
#ifdef M68K
struct sigcontext
{
	unsigned long sc_mask;
	unsigned long sc_usp;
	unsigned long sc_d0;
	unsigned long sc_d1;
	unsigned long sc_a0;
	unsigned long sc_a1;
	unsigned short sc_sr;
	unsigned long sc_pc;
	unsigned short sc_formatvec;
};
#endif /* M68K */
#endif /* !I386 */
#endif /* !HAVE_ASM_SIGCONTEXT_H */
#ifndef NSIG
#define NSIG 32
#endif
#ifdef ARM
#undef NSIG
#define NSIG 32
#endif
#endif /* LINUX */

char *signalent0[] = {
#include "signalent.h"
};
int nsignals0 = sizeof signalent0 / sizeof signalent0[0];

#if SUPPORTED_PERSONALITIES >= 2
char *signalent1[] = {
#include "signalent1.h"
};
int nsignals1 = sizeof signalent1 / sizeof signalent1[0];
#endif /* SUPPORTED_PERSONALITIES >= 2 */

#if SUPPORTED_PERSONALITIES >= 3
char *signalent2[] = {
#include "signalent2.h"
};
int nsignals2 = sizeof signalent2 / sizeof signalent2[0];
#endif /* SUPPORTED_PERSONALITIES >= 3 */

char **signalent;
int nsignals;

#ifdef SUNOS4

static struct xlat sigvec_flags[] = {
	{ SV_ONSTACK,	"SV_ONSTACK"	},
	{ SV_INTERRUPT,	"SV_INTERRUPT"	},
	{ SV_RESETHAND,	"SV_RESETHAND"	},
	{ SA_NOCLDSTOP,	"SA_NOCLDSTOP"	},
	{ 0,		NULL		},
};

#endif /* SUNOS4 */

#ifdef HAVE_SIGACTION

static struct xlat sigact_flags[] = {
#ifdef SA_STACK
	{ SA_STACK,	"SA_STACK"	},
#endif
#ifdef SA_RESTART
	{ SA_RESTART,	"SA_RESTART"	},
#endif
#ifdef SA_INTERRUPT
	{ SA_INTERRUPT,	"SA_INTERRUPT"	},
#endif
#ifdef SA_NOMASK
	{ SA_NOMASK,	"SA_NOMASK"	},
#endif
#ifdef SA_ONESHOT
	{ SA_ONESHOT,	"SA_ONESHOT"	},
#endif
#ifdef SA_SIGINFO
	{ SA_SIGINFO,	"SA_SIGINFO"	},
#endif
#ifdef SA_RESETHAND
	{ SA_RESETHAND,	"SA_RESETHAND"	},
#endif
#ifdef SA_ONSTACK
	{ SA_ONSTACK,	"SA_ONSTACK"	},
#endif
#ifdef SA_NODEFER
	{ SA_NODEFER,	"SA_NODEFER"	},
#endif
#ifdef SA_NOCLDSTOP
	{ SA_NOCLDSTOP,	"SA_NOCLDSTOP"	},
#endif
#ifdef SA_NOCLDWAIT
	{ SA_NOCLDWAIT,	"SA_NOCLDWAIT"	},
#endif
#ifdef _SA_BSDCALL
	{ _SA_BSDCALL,	"_SA_BSDCALL"	},
#endif
	{ 0,		NULL		},
};

static struct xlat sigprocmaskcmds[] = {
	{ SIG_BLOCK,	"SIG_BLOCK"	},
	{ SIG_UNBLOCK,	"SIG_UNBLOCK"	},
	{ SIG_SETMASK,	"SIG_SETMASK"	},
#ifdef SIG_SETMASK32
	{ SIG_SETMASK32,"SIG_SETMASK32"	},
#endif
	{ 0,		NULL		},
};

#endif /* HAVE_SIGACTION */

/* Anonymous realtime signals. */
/* Under glibc 2.1, SIGRTMIN et al are functions, but __SIGRTMIN is a
   constant.  This is what we want.  Otherwise, just use SIGRTMIN. */
#ifdef SIGRTMIN
#ifndef __SIGRTMIN
#define __SIGRTMIN SIGRTMIN
#define __SIGRTMAX SIGRTMAX /* likewise */
#endif
#endif

char *
signame(sig)
int sig;
{
	static char buf[30];
	if (sig < nsignals) {
		return signalent[sig];
#ifdef SIGRTMIN
	} else if (sig >= __SIGRTMIN && sig <= __SIGRTMAX) {
		sprintf(buf, "SIGRT_%ld", (long)(sig - __SIGRTMIN));
		return buf;
#endif /* SIGRTMIN */
	} else {
		sprintf(buf, "%d", sig);
		return buf;
	}
}

#ifndef UNIXWARE
static void
long_to_sigset(l, s)
long l;
sigset_t *s;
{
	sigemptyset(s);
	*(long *)s = l;
}
#endif

static int
copy_sigset_len(tcp, addr, s, len)
struct tcb *tcp;
long addr;
sigset_t *s;
int len;
{
	if (len > sizeof(*s))
		len = sizeof(*s);
	sigemptyset(s);
	if (umoven(tcp, addr, len, (char *)s) < 0)
		return -1;
	return 0;
}

#ifdef LINUX
/* Original sigset is unsigned long */
#define copy_sigset(tcp, addr, s) copy_sigset_len(tcp, addr, s, sizeof(long))
#else
#define copy_sigset(tcp, addr, s) copy_sigset_len(tcp, addr, s, sizeof(sigset_t))
#endif

static char *
sprintsigmask(s, mask, rt)
char *s;
sigset_t *mask;
int rt; /* set might include realtime sigs */
{
	int i, nsigs;
	int maxsigs;
	char *format;
	static char outstr[256];

	strcpy(outstr, s);
	s = outstr + strlen(outstr);
	nsigs = 0;
	maxsigs = nsignals;
#ifdef __SIGRTMAX
	if (rt)
		maxsigs = __SIGRTMAX; /* instead */
#endif
	for (i = 1; i < maxsigs; i++) {
		if (sigismember(mask, i) == 1)
			nsigs++;
	}
	if (nsigs >= nsignals * 2 / 3) {
		*s++ = '~';
		for (i = 1; i < maxsigs; i++) {
			switch (sigismember(mask, i)) {
			case 1:
				sigdelset(mask, i);
				break;
			case 0:
				sigaddset(mask, i);
				break;
			}
		}
	}
	format = "%s";
	*s++ = '[';
	for (i = 1; i < maxsigs; i++) {
		if (sigismember(mask, i) == 1) {
			sprintf(s, format, signame(i) + 3); s += strlen(s);
			format = " %s";
		}
	}
	*s++ = ']';
	*s = '\0';
	return outstr;
}

static void
printsigmask(mask, rt)
sigset_t *mask;
int rt;
{
	tprintf("%s", sprintsigmask("", mask, rt));
}

void
printsignal(nr)
int nr;
{
	tprintf(signame(nr));
}

/*
 * Check process TCP for the disposition of signal SIG.
 * Return 1 if the process would somehow manage to  survive signal SIG,
 * else return 0.  This routine will never be called with SIGKILL.
 */
int
sigishandled(tcp, sig)
struct tcb *tcp;
int sig;
{
#ifdef LINUX
	int sfd;
	char sname[32];
	char buf[1024];
	char *s;
	int i;
	unsigned int signalled, blocked, ignored, caught;

	/* This is incredibly costly but it's worth it. */
	sprintf(sname, "/proc/%d/stat", tcp->pid);
	if ((sfd = open(sname, O_RDONLY)) == -1) {
		perror(sname);
		return 1;
	}
	i = read(sfd, buf, 1024);
	buf[i] = '\0';
	close(sfd);
	/*
	 * Skip the extraneous fields. This loses if the
	 * command name has any spaces in it.  So be it.
	 */
	for (i = 0, s = buf; i < 30; i++) {
		while (*++s != ' ') {
			if (!*s)
				break;
		}
	}
	if (sscanf(s, "%u%u%u%u",
		   &signalled, &blocked, &ignored, &caught) != 4) {
		fprintf(stderr, "/proc/pid/stat format error\n");
		return 1;
	}
#ifdef DEBUG
	fprintf(stderr, "sigs: %08x %08x %08x %08x\n",
		signalled, blocked, ignored, caught);
#endif
	if ((ignored & sigmask(sig)) || (caught & sigmask(sig)))
		return 1;
#endif /* LINUX */

#ifdef SUNOS4
	void (*u_signal)();

	if (upeek(tcp->pid, uoff(u_signal[0]) + sig*sizeof(u_signal),
	    (long *) &u_signal) < 0) {
		return 0;
	}
	if (u_signal != SIG_DFL)
		return 1;
#endif /* SUNOS4 */

#ifdef SVR4
	/*
	 * Since procfs doesn't interfere with wait I think it is safe
	 * to punt on this question.  If not, the information is there.
	 */
	return 1;
#else /* !SVR4 */
	switch (sig) {
	case SIGCONT:
	case SIGSTOP:
	case SIGTSTP:
	case SIGTTIN:
	case SIGTTOU:
	case SIGCHLD:
	case SIGIO:
#if defined(SIGURG) && SIGURG != SIGIO
	case SIGURG:
#endif
	case SIGWINCH:
		/* Gloria Gaynor says ... */
		return 1;
	default:
		break;
	}
	return 0;
#endif /* !SVR4 */
}

#if defined(SUNOS4)

int
sys_sigvec(tcp)
struct tcb *tcp;
{
	struct sigvec sv;
	long addr;

	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprintf(", ");
		addr = tcp->u_arg[1];
	} else {
		addr = tcp->u_arg[2];
	}
	if (addr == 0)
		tprintf("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &sv) < 0)
		tprintf("{...}");
	else {
		switch ((int) sv.sv_handler) {
		case (int) SIG_ERR:
			tprintf("{SIG_ERR}");
			break;
		case (int) SIG_DFL:
			tprintf("{SIG_DFL}");
			break;
		case (int) SIG_IGN:
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
			tprintf("{SIG_IGN}");
			break;
		case (int) SIG_HOLD:
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
			tprintf("SIG_HOLD");
			break;
		default:
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
			tprintf("{%#lx, ", (unsigned long) sv.sv_handler);
			printsigmask(&sv.sv_mask, 0);
			tprintf(", ");
			if (!printflags(sigvec_flags, sv.sv_flags))
				tprintf("0");
			tprintf("}");
		}
	}
	if (entering(tcp))
		tprintf(", ");
	return 0;
}

int
sys_sigpause(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {	/* WTA: UD had a bug here: he forgot the braces */
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[0], &sigm);
		printsigmask(&sigm, 0);
	}
	return 0;
}

int
sys_sigstack(tcp)
struct tcb *tcp;
{
	struct sigstack ss;
	long addr;

	if (entering(tcp))
		addr = tcp->u_arg[0];
	else
		addr = tcp->u_arg[1];
	if (addr == 0)
		tprintf("NULL");
	else if (umove(tcp, addr, &ss) < 0)
		tprintf("%#lx", addr);
	else {
		tprintf("{ss_sp %#lx ", (unsigned long) ss.ss_sp);
		tprintf("ss_onstack %s}", ss.ss_onstack ? "YES" : "NO");
	}
	if (entering(tcp))
		tprintf(", ");
	return 0;
}

int
sys_sigcleanup(tcp)
struct tcb *tcp;
{
	return 0;
}

#endif /* SUNOS4 */

#ifndef SVR4

int
sys_sigsetmask(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[0], &sigm);
		printsigmask(&sigm, 0);
		if ((tcp->u_arg[0] & sigmask(SIGTRAP))) {
			/* Mark attempt to block SIGTRAP */
			tcp->flags |= TCB_SIGTRAPPED;
			/* Send unblockable signal */
			kill(tcp->pid, SIGSTOP);
		}
	}
	else if (!syserror(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_rval, &sigm);
		tcp->auxstr = sprintsigmask("old mask ", &sigm, 0);

		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

int
sys_sigblock(tcp)
struct tcb *tcp;
{
	return sys_sigsetmask(tcp);
}

#endif /* !SVR4 */

#ifdef HAVE_SIGACTION

#ifdef LINUX
struct old_sigaction {
	__sighandler_t __sa_handler;
	unsigned long sa_mask;
	unsigned long sa_flags;
	void (*sa_restorer)(void);
};
#define SA_HANDLER __sa_handler
#endif /* LINUX */

#ifndef SA_HANDLER                                                           
#define SA_HANDLER sa_handler                                                
#endif

int
sys_sigaction(tcp)
struct tcb *tcp;
{
	long addr;
	sigset_t sigset;
#ifdef LINUX
	struct old_sigaction sa;
#else
	struct sigaction sa;
#endif


	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprintf(", ");
		addr = tcp->u_arg[1];
	} else
		addr = tcp->u_arg[2];
	if (addr == 0)
		tprintf("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &sa) < 0)
		tprintf("{...}");
	else {
		switch ((long) sa.SA_HANDLER) {
		case (long) SIG_ERR:
			tprintf("{SIG_ERR}");
			break;
		case (long) SIG_DFL:
			tprintf("{SIG_DFL}");
			break;
		case (long) SIG_IGN:
#ifndef SVR4
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
#endif /* !SVR4 */
			tprintf("{SIG_IGN}");
			break;
		default:
#ifndef SVR4
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
#endif /* !SVR4 */
			tprintf("{%#lx, ", (long) sa.SA_HANDLER);
#ifndef LINUX
			printsigmask (&sa.sa_mask, 0);
#else
			long_to_sigset(sa.sa_mask, &sigset);
			printsigmask(&sigset, 0);
#endif
			tprintf(", ");
			if (!printflags(sigact_flags, sa.sa_flags))
				tprintf("0");
			tprintf("}");
		}
	}
	if (entering(tcp))
		tprintf(", ");
#ifdef LINUX
	else
		tprintf(", %#lx", (unsigned long) sa.sa_restorer);
#endif
	return 0;
}

int
sys_signal(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		switch (tcp->u_arg[1]) {
		case (int) SIG_ERR:
			tprintf("SIG_ERR");
			break;
		case (int) SIG_DFL:
			tprintf("SIG_DFL");
			break;
		case (int) SIG_IGN:
#ifndef SVR4
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
#endif /* !SVR4 */
			tprintf("SIG_IGN");
			break;
		default:
#ifndef SVR4
			if (tcp->u_arg[0] == SIGTRAP) {
				tcp->flags |= TCB_SIGTRAPPED;
				kill(tcp->pid, SIGSTOP);
			}
#endif /* !SVR4 */
			tprintf("%#lx", tcp->u_arg[1]);
		}
	}
	return 0;
}

#endif /* HAVE_SIGACTION */

#ifdef LINUX

int
sys_sigreturn(tcp)
struct tcb *tcp;
{
#ifdef S390
    long usp;
    struct sigcontext_struct sc;

    if (entering(tcp)) {
	    tcp->u_arg[0] = 0;
	    if (upeek(tcp->pid,PT_GPR15,&usp)<0)
		    return 0;
	    if (umove(tcp, usp+__SIGNAL_FRAMESIZE, &sc) < 0)
		    return 0;
	    tcp->u_arg[0] = 1;
	    memcpy(&tcp->u_arg[1],&sc.oldmask[0],sizeof(sigset_t));
    } else {
	    tcp->u_rval = tcp->u_error = 0;
	    if (tcp->u_arg[0] == 0)
		    return 0;
	    tcp->auxstr = sprintsigmask("mask now ",(sigset_t *)&tcp->u_arg[1]);
	    return RVAL_NONE | RVAL_STR;
    }
    return 0;
#else
#ifdef I386
	long esp;
	struct sigcontext_struct sc;

	if (entering(tcp)) {
		tcp->u_arg[0] = 0;
		if (upeek(tcp->pid, 4*UESP, &esp) < 0)
			return 0;
		if (umove(tcp, esp, &sc) < 0)
			return 0;
		tcp->u_arg[0] = 1;
		tcp->u_arg[1] = sc.oldmask;
	}
	else {
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[1], &sigm);
		tcp->u_rval = tcp->u_error = 0;
		if (tcp->u_arg[0] == 0)
			return 0;
		tcp->auxstr = sprintsigmask("mask now ", &sigm, 0);
		return RVAL_NONE | RVAL_STR;
	}
	return 0;
#else /* !I386 */
#ifdef IA64
	struct sigcontext sc;
	long sp;

	if (entering(tcp)) {
		tcp->u_arg[0] = 0;
		if (upeek(tcp->pid, PT_R12, &sp) < 0)
			return 0;
		if (umove(tcp, sp + 16, &sc) < 0)
			return 0;
		tcp->u_arg[0] = 1;
		memcpy(tcp->u_arg + 1, &sc.sc_mask, sizeof(tcp->u_arg[1]));
	}
	else {
		tcp->u_rval = tcp->u_error = 0;
		if (tcp->u_arg[0] == 0)
			return 0;
		tcp->auxstr = sprintsigmask("mask now ", tcp->u_arg[1]);
		return RVAL_NONE | RVAL_STR;
	}
	return 0;
#else /* !IA64 */
#ifdef POWERPC
       long esp;
       struct sigcontext_struct sc;

       if (entering(tcp)) {
		   tcp->u_arg[0] = 0;
		   if (upeek(tcp->pid, 4*PT_R1, &esp) < 0)
			   return 0;
		   if (umove(tcp, esp, &sc) < 0)
			   return 0;
		   tcp->u_arg[0] = 1;
		   tcp->u_arg[1] = sc.oldmask;
       }
       else {
		   sigset_t sigm;
		   long_to_sigset(tcp->u_arg[1], &sigm);
		   tcp->u_rval = tcp->u_error = 0;
		   if (tcp->u_arg[0] == 0)
			   return 0;
		   tcp->auxstr = sprintsigmask("mask now ", &sigm, 0);
		   return RVAL_NONE | RVAL_STR;
       }
       return 0;
#else /* !POWERPC */
#ifdef M68K
	long usp;
	struct sigcontext sc;

	if (entering(tcp)) {
	    tcp->u_arg[0] = 0;
	    if (upeek(tcp->pid, 4*PT_USP, &usp) < 0)
			return 0;
	    if (umove(tcp, usp, &sc) < 0)
			return 0;
	    tcp->u_arg[0] = 1;
	    tcp->u_arg[1] = sc.sc_mask;
	}
	else {
	    sigset_t sigm;
	    long_to_sigset(tcp->u_arg[1], &sigm);
	    tcp->u_rval = tcp->u_error = 0;
	    if (tcp->u_arg[0] == 0)
			return 0;
	    tcp->auxstr = sprintsigmask("mask now ", &sigm, 0);
	    return RVAL_NONE | RVAL_STR;
	}
	return 0;
#else /* !M68K */
#ifdef ALPHA
	long fp;
	struct sigcontext_struct sc;

	if (entering(tcp)) {
	    tcp->u_arg[0] = 0;
	    if (upeek(tcp->pid, REG_FP, &fp) < 0)
			return 0;
	    if (umove(tcp, fp, &sc) < 0)
			return 0;
	    tcp->u_arg[0] = 1;
	    tcp->u_arg[1] = sc.sc_mask;
	}
	else {
	    sigset_t sigm;
	    long_to_sigset(tcp->u_arg[1], &sigm);
	    tcp->u_rval = tcp->u_error = 0;
	    if (tcp->u_arg[0] == 0)
			return 0;
	    tcp->auxstr = sprintsigmask("mask now ", &sigm, 0);
	    return RVAL_NONE | RVAL_STR;
	}
	return 0;
#else
#ifdef SPARC
	long i1;
	struct regs regs;
	m_siginfo_t si;

	if(ptrace(PTRACE_GETREGS, tcp->pid, (char *)&regs, 0) < 0) {
	    perror("sigreturn: PTRACE_GETREGS ");
	    return 0;
	}
	if(entering(tcp)) {
		tcp->u_arg[0] = 0;
		i1 = regs.r_o1;
		if(umove(tcp, i1, &si) < 0) {
			perror("sigreturn: umove ");
			return 0;
		}
		tcp->u_arg[0] = 1;
		tcp->u_arg[1] = si.si_mask;
	} else {
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[1], &sigm);
		tcp->u_rval = tcp->u_error = 0;
		if(tcp->u_arg[0] == 0)
			return 0;
		tcp->auxstr = sprintsigmask("mask now ", &sigm, 0);
		return RVAL_NONE | RVAL_STR;
	}
	return 0;
#else  
#ifdef MIPS
	long sp;
	struct sigcontext sc;

	if(entering(tcp)) {
	  	tcp->u_arg[0] = 0;
		if (upeek(tcp->pid, REG_SP, &sp) < 0)
		  	return 0;
		if (umove(tcp, sp, &sc) < 0)
		  	return 0;
		tcp->u_arg[0] = 1;
		tcp->u_arg[1] = sc.sc_sigset;
	} else {
	  	tcp->u_rval = tcp->u_error = 0;
		if(tcp->u_arg[0] == 0)
		  	return 0;
		tcp->auxstr = sprintsigmask("mask now ", tcp->u_arg[1]);
		return RVAL_NONE | RVAL_STR;
	}
	return 0;
#endif /* MIPS */
#endif /* SPARC */
#endif /* ALPHA */
#endif /* !M68K */
#endif /* !POWERPC */
#endif /* !IA64 */
#endif /* !I386 */
#endif /* S390 */
}

int
sys_siggetmask(tcp)
struct tcb *tcp;
{
	if (exiting(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_rval, &sigm);
		tcp->auxstr = sprintsigmask("mask ", &sigm, 0);
	}
	return RVAL_HEX | RVAL_STR;
}

int
sys_sigsuspend(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		sigset_t sigm;
		long_to_sigset(tcp->u_arg[2], &sigm);
#if 0
		/* first two are not really arguments, but print them anyway */
		/* nevermind, they are an anachronism now, too bad... */
		tprintf("%d, %#x, ", tcp->u_arg[0], tcp->u_arg[1]);
#endif
		printsigmask(&sigm, 0);
	}
	return 0;
}

#endif /* LINUX */

#ifdef SVR4

int
sys_sigsuspend(tcp)
struct tcb *tcp;
{
	sigset_t sigset;

	if (entering(tcp)) {
		if (umove(tcp, tcp->u_arg[0], &sigset) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigset, 0);
	}
	return 0;
}
static struct xlat ucontext_flags[] = {
	{ UC_SIGMASK,	"UC_SIGMASK"	},
	{ UC_STACK,	"UC_STACK"	},
	{ UC_CPU,	"UC_CPU"	},
#ifdef UC_FPU
	{ UC_FPU,	"UC_FPU"	},
#endif
#ifdef UC_INTR
	{ UC_INTR,	"UC_INTR"	},
#endif
	{ 0,		NULL		},
};

#endif

#if defined SVR4 || defined LINUX
#if defined LINUX && !defined SS_ONSTACK
#define SS_ONSTACK      1
#define SS_DISABLE      2
#if __GLIBC_MINOR__ == 0
typedef struct
{
	__ptr_t ss_sp;
	int ss_flags;
	size_t ss_size;
} stack_t;
#endif
#endif

static struct xlat sigaltstack_flags[] = {
	{ SS_ONSTACK,	"SS_ONSTACK"	},
	{ SS_DISABLE,	"SS_DISABLE"	},
	{ 0,		NULL		},
};
#endif

#ifdef SVR4
static void
printcontext(tcp, ucp)
struct tcb *tcp;
ucontext_t *ucp;
{
	tprintf("{");
	if (!abbrev(tcp)) {
		tprintf("uc_flags=");
		if (!printflags(ucontext_flags, ucp->uc_flags))
			tprintf("0");
		tprintf(", uc_link=%#lx, ", (unsigned long) ucp->uc_link);
	}
	tprintf("uc_sigmask=");
	printsigmask(ucp->uc_sigmask, 0);
	if (!abbrev(tcp)) {
		tprintf(", uc_stack={ss_sp=%#lx, ss_size=%d, ss_flags=",
			(unsigned long) ucp->uc_stack.ss_sp,
			ucp->uc_stack.ss_size);
		if (!printflags(sigaltstack_flags, ucp->uc_stack.ss_flags))
			tprintf("0");
		tprintf("}");
	}
	tprintf(", ...}");
}

int
sys_getcontext(tcp)
struct tcb *tcp;
{
	ucontext_t uc;

	if (entering(tcp)) {
		if (!tcp->u_arg[0])
			tprintf("NULL");
		else if (umove(tcp, tcp->u_arg[0], &uc) < 0)
			tprintf("{...}");
		else
			printcontext(tcp, &uc);
	}
	return 0;
}

int
sys_setcontext(tcp)
struct tcb *tcp;
{
	ucontext_t uc;

	if (entering(tcp)) {
		if (!tcp->u_arg[0])
			tprintf("NULL");
		else if (umove(tcp, tcp->u_arg[0], &uc) < 0)
			tprintf("{...}");
		else
			printcontext(tcp, &uc);
	}
	else {
		tcp->u_rval = tcp->u_error = 0;
		if (tcp->u_arg[0] == 0)
			return 0;
		return RVAL_NONE;
	}
	return 0;
}

#endif /* SVR4 */

#ifdef LINUX

static int
print_stack_t(tcp, addr)
struct tcb *tcp;
unsigned long addr;
{
	stack_t ss;
	if (umove(tcp, addr, &ss) < 0)
		return -1;
	tprintf("{ss_sp=%#lx, ss_flags=", (unsigned long) ss.ss_sp);
	if (!printflags(sigaltstack_flags, ss.ss_flags))
		tprintf("0");
	tprintf(", ss_size=%lu}", (unsigned long) ss.ss_size);
	return 0;
}

int
sys_sigaltstack(tcp)
	struct tcb *tcp;
{
	if (entering(tcp)) {
		if (tcp->u_arg[0] == 0)
			tprintf("NULL");
		else if (print_stack_t(tcp, tcp->u_arg[0]) < 0)
			return -1;
	}
	else {
		tprintf(", ");
		if (tcp->u_arg[1] == 0)
			tprintf("NULL");
		else if (print_stack_t(tcp, tcp->u_arg[1]) < 0)
			return -1;
	}
	return 0;
}
#endif

#ifdef HAVE_SIGACTION

int
sys_sigprocmask(tcp)
struct tcb *tcp;
{
#ifdef ALPHA
	if (entering(tcp)) {
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprintf(", ");
		printsigmask(tcp->u_arg[1], 0);
	}
	else if (!syserror(tcp)) {
		tcp->auxstr = sprintsigmask("old mask ", tcp->u_rval, 0);
		return RVAL_HEX | RVAL_STR;
	}
#else /* !ALPHA */
	sigset_t sigset;

	if (entering(tcp)) {
#ifdef SVR4
		if (tcp->u_arg[0] == 0)
			tprintf("0");
		else
#endif /* SVR4 */
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprintf(", ");
		if (!tcp->u_arg[1])
			tprintf("NULL, ");
		else if (copy_sigset(tcp, tcp->u_arg[1], &sigset) < 0)
			tprintf("%#lx, ", tcp->u_arg[1]);
		else {
			printsigmask(&sigset, 0);
			tprintf(", ");
		}
	}
	else {
		if (!tcp->u_arg[2])
			tprintf("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else if (copy_sigset(tcp, tcp->u_arg[2], &sigset) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigset, 0);
	}
#endif /* !ALPHA */
	return 0;
}

#endif /* HAVE_SIGACTION */

int
sys_kill(tcp)
struct tcb *tcp;
{
	if (entering(tcp)) {
		tprintf("%ld, %s", tcp->u_arg[0], signame(tcp->u_arg[1]));
	}
	return 0;
}

int
sys_killpg(tcp)
struct tcb *tcp;
{
	return sys_kill(tcp);
}

int
sys_sigpending(tcp)
struct tcb *tcp;
{
	sigset_t sigset;

	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (copy_sigset(tcp, tcp->u_arg[0], &sigset) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigset, 0);
	}
	return 0;
}

#ifdef LINUX

	int
sys_rt_sigprocmask(tcp)
	struct tcb *tcp;
{
	sigset_t sigset;

	/* Note: arg[3] is the length of the sigset. */
	if (entering(tcp)) {
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprintf(", ");
		if (!tcp->u_arg[1])
			tprintf("NULL, ");
		else if (copy_sigset_len(tcp, tcp->u_arg[1], &sigset, tcp->u_arg[3]) < 0)
			tprintf("%#lx, ", tcp->u_arg[1]);
		else {
			printsigmask(&sigset, 1);
			tprintf(", ");
		}
	}
	else {
		if (!tcp->u_arg[2])

			tprintf("NULL");
		else if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else if (copy_sigset_len(tcp, tcp->u_arg[2], &sigset, tcp->u_arg[3]) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigset, 1);
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}

#if __GLIBC_MINOR__ < 1
/* Type for data associated with a signal.  */
typedef union sigval
{
	int sival_int;
	void *sival_ptr;
} sigval_t;

# define __SI_MAX_SIZE     128
# define __SI_PAD_SIZE     ((__SI_MAX_SIZE / sizeof (int)) - 3)

typedef struct siginfo
{
	int si_signo;               /* Signal number.  */
	int si_errno;               /* If non-zero, an errno value associated with
								   this signal, as defined in <errno.h>.  */
	int si_code;                /* Signal code.  */

	union
	{
		int _pad[__SI_PAD_SIZE];

		/* kill().  */
		struct
		{
			__pid_t si_pid;     /* Sending process ID.  */
			__uid_t si_uid;     /* Real user ID of sending process.  */
		} _kill;

		/* POSIX.1b timers.  */
		struct
		{
			unsigned int _timer1;
			unsigned int _timer2;
		} _timer;

		/* POSIX.1b signals.  */
		struct
		{
			__pid_t si_pid;     /* Sending process ID.  */
			__uid_t si_uid;     /* Real user ID of sending process.  */
			sigval_t si_sigval; /* Signal value.  */
		} _rt;

		/* SIGCHLD.  */
		struct
		{
			__pid_t si_pid;     /* Which child.  */
			int si_status;      /* Exit value or signal.  */
			__clock_t si_utime;
			__clock_t si_stime;
		} _sigchld;

		/* SIGILL, SIGFPE, SIGSEGV, SIGBUS.  */
		struct
		{
			void *si_addr;      /* Faulting insn/memory ref.  */
		} _sigfault;

		/* SIGPOLL.  */
		struct
		{
			int si_band;        /* Band event for SIGPOLL.  */
			int si_fd;
		} _sigpoll;
	} _sifields;
} siginfo_t;
#endif

/* Structure describing the action to be taken when a signal arrives.  */
struct new_sigaction
{
	union
	{
		__sighandler_t __sa_handler;
		void (*__sa_sigaction) (int, siginfo_t *, void *);
	}
	__sigaction_handler;
	unsigned long sa_flags;
	void (*sa_restorer) (void);
	unsigned long int sa_mask[2];
};


	int
sys_rt_sigaction(tcp)
	struct tcb *tcp;
{
	struct new_sigaction sa;
	sigset_t sigset;
	long addr;

	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprintf(", ");
		addr = tcp->u_arg[1];
	} else
		addr = tcp->u_arg[2];
	if (addr == 0)
		tprintf("NULL");
	else if (!verbose(tcp))
		tprintf("%#lx", addr);
	else if (umove(tcp, addr, &sa) < 0)
		tprintf("{...}");
	else {
		switch ((long) sa.__sigaction_handler.__sa_handler) {
			case (long) SIG_ERR:
				tprintf("{SIG_ERR}");
				break;
			case (long) SIG_DFL:
				tprintf("{SIG_DFL}");
				break;
			case (long) SIG_IGN:
				tprintf("{SIG_IGN}");
				break;
			default:
				tprintf("{%#lx, ",
						(long) sa.__sigaction_handler.__sa_handler);
				sigemptyset(&sigset);
#ifdef LINUXSPARC
				if (tcp->u_arg[4] <= sizeof(sigset))
					memcpy(&sigset, &sa.sa_mask, tcp->u_arg[4]);
#else
				if (tcp->u_arg[3] <= sizeof(sigset))
					memcpy(&sigset, &sa.sa_mask, tcp->u_arg[3]);
#endif
				else
					memcpy(&sigset, &sa.sa_mask, sizeof(sigset));
				printsigmask(&sigset, 1);
				tprintf(", ");
				if (!printflags(sigact_flags, sa.sa_flags))
					tprintf("0");
				tprintf("}");
		}
	}
	if (entering(tcp))
		tprintf(", ");
	else
#ifdef LINUXSPARC
		tprintf(", %#lx, %lu", tcp->u_arg[3], tcp->u_arg[4]);
#elif defined(ALPHA)
		tprintf(", %lu, %#lx", tcp->u_arg[3], tcp->u_arg[4]);
#else
		tprintf(", %lu", addr = tcp->u_arg[3]);
#endif
	return 0;
}

	int
sys_rt_sigpending(tcp)
	struct tcb *tcp;
{
	sigset_t sigset;

	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else if (copy_sigset_len(tcp, tcp->u_arg[0],
					 &sigset, tcp->u_arg[1]) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigset, 1);
	}
	return 0;
}
	int
sys_rt_sigsuspend(tcp)
	struct tcb *tcp;
{
	if (entering(tcp)) {
		sigset_t sigm;
		if (copy_sigset_len(tcp, tcp->u_arg[0], &sigm, tcp->u_arg[1]) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigm, 1);
	}
	return 0;
}
#ifndef ILL_ILLOPC
#define ILL_ILLOPC      1       /* illegal opcode */
#define ILL_ILLOPN      2       /* illegal operand */
#define ILL_ILLADR      3       /* illegal addressing mode */
#define ILL_ILLTRP      4       /* illegal trap */
#define ILL_PRVOPC      5       /* privileged opcode */
#define ILL_PRVREG      6       /* privileged register */
#define ILL_COPROC      7       /* coprocessor error */
#define ILL_BADSTK      8       /* internal stack error */
#define FPE_INTDIV      1       /* integer divide by zero */
#define FPE_INTOVF      2       /* integer overflow */
#define FPE_FLTDIV      3       /* floating point divide by zero */
#define FPE_FLTOVF      4       /* floating point overflow */
#define FPE_FLTUND      5       /* floating point underflow */
#define FPE_FLTRES      6       /* floating point inexact result */
#define FPE_FLTINV      7       /* floating point invalid operation */
#define FPE_FLTSUB      8       /* subscript out of range */
#define SEGV_MAPERR     1       /* address not mapped to object */
#define SEGV_ACCERR     2       /* invalid permissions for mapped object */
#define BUS_ADRALN      1       /* invalid address alignment */
#define BUS_ADRERR      2       /* non-existant physical address */
#define BUS_OBJERR      3       /* object specific hardware error */
#define TRAP_BRKPT      1       /* process breakpoint */
#define TRAP_TRACE      2       /* process trace trap */
#define CLD_EXITED      1       /* child has exited */
#define CLD_KILLED      2       /* child was killed */
#define CLD_DUMPED      3       /* child terminated abnormally */
#define CLD_TRAPPED     4       /* traced child has trapped */
#define CLD_STOPPED     5       /* child has stopped */
#define CLD_CONTINUED   6       /* stopped child has continued */
#define POLL_IN         1       /* data input available */
#define POLL_OUT        2       /* output buffers available */
#define POLL_MSG        3       /* input message available */
#define POLL_ERR        4       /* i/o error */
#define POLL_PRI        5       /* high priority input available */
#define POLL_HUP        6       /* device disconnected */
#define SI_USER         0       /* sent by kill, sigsend, raise */
#define SI_QUEUE        -1      /* sent by sigqueue */
#define SI_TIMER        -2      /* sent by timer expiration */
#define SI_MESGQ        -3      /* sent by real time mesq state change */
#define SI_ASYNCIO      -4      /* sent by AIO completion */
#else
#undef si_pid
#undef si_uid
#undef si_status
#undef si_utime
#undef si_stime
#undef si_value
#undef si_int
#undef si_ptr
#undef si_addr
#undef si_band
#undef si_fd
#endif

static struct xlat sigill_flags[] = {
	{ILL_ILLOPC, "ILL_ILLOPC"},
	{ILL_ILLOPN, "ILL_ILLOPN"},
	{ILL_ILLADR, "ILL_ILLADR"},
	{ILL_ILLTRP, "ILL_ILLTRP"},
	{ILL_PRVOPC, "ILL_PRVOPC"},
	{ILL_PRVREG, "ILL_PRVREG"},
	{ILL_COPROC, "ILL_COPROC"},
	{ILL_BADSTK, "ILL_BADSTK"},
	{0, NULL}
};

static struct xlat sigfpe_flags[] = {
	{FPE_INTDIV, "FPE_INTDIV"},
	{FPE_INTOVF, "FPE_INTOVF"},
	{FPE_FLTDIV, "FPE_FLTDIV"},
	{FPE_FLTOVF, "FPE_FLTOVF"},
	{FPE_FLTUND, "FPE_FLTUND"},
	{FPE_FLTRES, "FPE_FLTRES"},
	{FPE_FLTINV, "FPE_FLTINV"},
	{FPE_FLTSUB, "FPE_FLTSUB"},
	{0, NULL}
};

static struct xlat sigsegv_flags[] = {
	{SEGV_MAPERR, "SEGV_MAPERR"},
	{SEGV_ACCERR, "SEGV_ACCERR"},
	{0, NULL}
};

static struct xlat sigbus_flags[] = {
	{BUS_ADRALN, "BUS_ADRALN"},
	{BUS_ADRERR, "BUS_ADRERR"},
	{BUS_OBJERR, "BUS_OBJERR"},
	{0, NULL}
};

static struct xlat sigtrap_flags[] = {
	{TRAP_BRKPT, "TRAP_BRKPT"},
	{TRAP_TRACE, "TRAP_TRACE"},
	{0, NULL}
};

static struct xlat sigchld_flags[] = {
	{CLD_EXITED, "CLD_EXITED"},
	{CLD_KILLED, "CLD_KILLED"},
	{CLD_DUMPED, "CLD_DUMPED"},
	{CLD_TRAPPED, "CLD_TRAPPED"},
	{CLD_STOPPED, "CLD_STOPPED"},
	{CLD_CONTINUED, "CLD_CONTINUED"},
	{0, NULL}
};

static struct xlat sigpoll_flags[] = {
	{POLL_IN, "POLL_IN"},
	{POLL_OUT, "POLL_OUT"},
	{POLL_MSG, "POLL_MSG"},
	{POLL_ERR, "POLL_ERR"},
	{POLL_PRI, "POLL_PRI"},
	{POLL_HUP, "POLL_HUP"},
	{0, NULL}
};

static struct xlat siginfo_flags[] = {
	{SI_USER, "SI_USER"},
	{SI_QUEUE, "SI_QUEUE"},
	{SI_TIMER, "SI_TIMER"},
	{SI_MESGQ, "SI_MESGQ"},
	{SI_ASYNCIO, "SI_ASYNCIO"},
	{0, NULL}
};

	static void
printsiginfo(tcp, si)
	struct tcb *tcp;
	siginfo_t *si;
{
	tprintf("{si_signo=");
	printsignal(si->si_signo);
	tprintf(", si_errno=%d, si_code=", si->si_errno);
	switch(si->si_signo)
	{
		case SIGILL:
			if (!printflags(sigill_flags, si->si_code))
				tprintf("%d /* ILL_??? */", si->si_code);
			tprintf(", si_addr=%lx",
					(unsigned long) si->_sifields._sigfault.si_addr);
			break;
		case SIGFPE:
			if (!printflags(sigfpe_flags, si->si_code))
				tprintf("%d /* FPE_??? */", si->si_code);
			tprintf(", si_addr=%lx",
					(unsigned long) si->_sifields._sigfault.si_addr);
			break;
		case SIGSEGV:
			if (!printflags(sigsegv_flags, si->si_code))
				tprintf("%d /* SEGV_??? */", si->si_code);
			tprintf(", si_addr=%lx",
					(unsigned long) si->_sifields._sigfault.si_addr);
			break;
		case SIGBUS:
			if (!printflags(sigbus_flags, si->si_code))
				tprintf("%d /* BUS_??? */", si->si_code);
			tprintf(", si_addr=%lx",
					(unsigned long) si->_sifields._sigfault.si_addr);
			break;
		case SIGTRAP:
			if (!printflags(sigtrap_flags, si->si_code))
				tprintf("%d /* TRAP_??? */", si->si_code);
			break;
		case SIGCHLD:
			if (!printflags(sigchld_flags, si->si_code))
				tprintf("%d /* CLD_??? */", si->si_code);
			if (!verbose(tcp))
				tprintf(", ...");
			else
				tprintf(", si_pid=%d, si_uid=%d, si_status=%d, si_utime=%lu, si_stime=%lu",
						si->_sifields._kill.si_pid,
						si->_sifields._kill.si_uid,
						si->_sifields._sigchld.si_status,
						si->_sifields._sigchld.si_utime,
						si->_sifields._sigchld.si_stime);
			break;
		case SIGPOLL:
			if (!printflags(sigpoll_flags, si->si_code))
				tprintf("%d /* POLL_??? */", si->si_code);
			if (si->si_code == POLL_IN
					|| si->si_code == POLL_OUT
					|| si->si_code == POLL_MSG)
				tprintf(", si_bind=%lu, si_fd=%d",
						(unsigned long) si->_sifields._sigpoll.si_band,
						si->_sifields._sigpoll.si_fd);
			break;
		default:
			if (!printflags(siginfo_flags, si->si_code))
				tprintf("%d /* SI_??? */", si->si_code);
			tprintf(", si_pid=%lu, si_uid=%lu, si_value={",
					(unsigned long) si->_sifields._rt.si_pid,
					(unsigned long) si->_sifields._rt.si_uid);
			if (!verbose(tcp))
				tprintf("...");
			else {
				tprintf("sival_int=%u, sival_ptr=%#lx",
						si->_sifields._rt.si_sigval.sival_int,
						(unsigned long) si->_sifields._rt.si_sigval.sival_ptr);
			}
			tprintf("}");
			break;
	}
	tprintf("}");
}

	int
sys_rt_sigqueueinfo(tcp)
	struct tcb *tcp;
{
	if (entering(tcp)) {
		siginfo_t si;
		tprintf("%lu, ", tcp->u_arg[0]);
		printsignal(tcp->u_arg[1]);
		tprintf(", ");
		if (umove(tcp, tcp->u_arg[2], &si) < 0)
			tprintf("%#lx", tcp->u_arg[2]);
		else
			printsiginfo(&si);
	}
	return 0;
}

int sys_rt_sigtimedwait(tcp)
	struct tcb *tcp;
{
	if (entering(tcp)) {
		sigset_t sigset;

		if (copy_sigset_len(tcp, tcp->u_arg[0], 
				    &sigset, tcp->u_arg[3]) < 0)
			tprintf("[?]");
		else
			printsigmask(&sigset, 1);
		tprintf(", ");
	}
	else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else {
			siginfo_t si;
			if (umove(tcp, tcp->u_arg[1], &si) < 0)
				tprintf("%#lx", tcp->u_arg[1]);
			else
				printsiginfo(&si);
			/* XXX For now */
			tprintf(", %#lx", tcp->u_arg[2]);
			tprintf(", %d", (int) tcp->u_arg[3]);
		}
	}
	return 0;
};

#endif /* LINUX */

