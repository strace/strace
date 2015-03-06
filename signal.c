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
 */

#include "defs.h"

#ifndef NSIG
# warning NSIG is not defined, using 32
# define NSIG 32
#elif NSIG < 32
# error NSIG < 32
#endif

/* The libc headers do not define this constant since it should only be
   used by the implementation.  So we define it here.  */
#ifndef SA_RESTORER
# ifdef ASM_SA_RESTORER
#  define SA_RESTORER ASM_SA_RESTORER
# endif
#endif

/*
 * Some architectures define SA_RESTORER in their headers,
 * but do not actually have sa_restorer.
 *
 * Some architectures, otherwise, do not define SA_RESTORER in their headers,
 * but actually have sa_restorer.
 */
#ifdef SA_RESTORER
# if defined HPPA || defined IA64
#  define HAVE_SA_RESTORER 0
# else
#  define HAVE_SA_RESTORER 1
# endif
#else /* !SA_RESTORER */
# if defined SPARC || defined SPARC64
#  define HAVE_SA_RESTORER 1
# else
#  define HAVE_SA_RESTORER 0
# endif
#endif

#include "xlat/sigact_flags.h"
#include "xlat/sigprocmaskcmds.h"

/* Anonymous realtime signals. */
#ifndef ASM_SIGRTMIN
/* Linux kernel >= 3.18 defines SIGRTMIN to 32 on all architectures. */
# define ASM_SIGRTMIN 32
#endif
#ifndef ASM_SIGRTMAX
/* Under glibc 2.1, SIGRTMAX et al are functions, but __SIGRTMAX is a
   constant.  This is what we want.  Otherwise, just use SIGRTMAX. */
# ifdef SIGRTMAX
#  ifndef __SIGRTMAX
#   define __SIGRTMAX SIGRTMAX
#  endif
# endif
# ifdef __SIGRTMAX
#  define ASM_SIGRTMAX __SIGRTMAX
# endif
#endif

/* Note on the size of sigset_t:
 *
 * In glibc, sigset_t is an array with space for 1024 bits (!),
 * even though all arches supported by Linux have only 64 signals
 * except MIPS, which has 128. IOW, it is 128 bytes long.
 *
 * In-kernel sigset_t is sized correctly (it is either 64 or 128 bit long).
 * However, some old syscall return only 32 lower bits (one word).
 * Example: sys_sigpending vs sys_rt_sigpending.
 *
 * Be aware of this fact when you try to
 *     memcpy(&tcp->u_arg[1], &something, sizeof(sigset_t))
 * - sizeof(sigset_t) is much bigger than you think,
 * it may overflow tcp->u_arg[] array, and it may try to copy more data
 * than is really available in <something>.
 * Similarly,
 *     umoven(tcp, addr, sizeof(sigset_t), &sigset)
 * may be a bad idea: it'll try to read much more data than needed
 * to fetch a sigset_t.
 * Use (NSIG / 8) as a size instead.
 */

const char *
signame(const int sig)
{
	static char buf[sizeof("SIGRT_%u") + sizeof(int)*3];

	if (sig >= 0) {
		const unsigned int s = sig;

		if (s < nsignals)
			return signalent[s];
#ifdef ASM_SIGRTMAX
		if (s >= ASM_SIGRTMIN && s <= ASM_SIGRTMAX) {
			sprintf(buf, "SIGRT_%u", s - ASM_SIGRTMIN);
			return buf;
		}
#endif
	}
	sprintf(buf, "%d", sig);
	return buf;
}

static unsigned int
popcount32(const uint32_t *a, unsigned int size)
{
	unsigned int count = 0;

	for (; size; ++a, --size) {
		uint32_t x = *a;

#ifdef HAVE___BUILTIN_POPCOUNT
		count += __builtin_popcount(x);
#else
		for (; x; ++count)
			x &= x - 1;
#endif
	}

	return count;
}

const char *
sprintsigmask_n(const char *prefix, const void *sig_mask, unsigned int bytes)
{
	/*
	 * The maximum number of signal names to be printed is NSIG * 2 / 3.
	 * Most of signal names have length 7,
	 * average length of signal names is less than 7.
	 * The length of prefix string does not exceed 16.
	 */
	static char outstr[128 + 8 * (NSIG * 2 / 3)];

	char *s;
	const uint32_t *mask;
	uint32_t inverted_mask[NSIG / 32];
	unsigned int size;
	int i;
	char sep;

	s = stpcpy(outstr, prefix);

	mask = sig_mask;
	/* length of signal mask in 4-byte words */
	size = (bytes >= NSIG / 8) ? NSIG / 32 : (bytes + 3) / 4;

	/* check whether 2/3 or more bits are set */
	if (popcount32(mask, size) >= size * 32 * 2 / 3) {
		/* show those signals that are NOT in the mask */
		unsigned int j;
		for (j = 0; j < size; ++j)
			inverted_mask[j] = ~mask[j];
		mask = inverted_mask;
		*s++ = '~';
	}

	sep = '[';
	for (i = 0; (i = next_set_bit(mask, i, size * 32)) >= 0; ) {
		++i;
		*s++ = sep;
		if ((unsigned) i < nsignals) {
			s = stpcpy(s, signalent[i] + 3);
		}
#ifdef ASM_SIGRTMAX
		else if (i >= ASM_SIGRTMIN && i <= ASM_SIGRTMAX) {
			s += sprintf(s, "RT_%u", i - ASM_SIGRTMIN);
		}
#endif
		else {
			s += sprintf(s, "%u", i);
		}
		sep = ' ';
	}
	if (sep == '[')
		*s++ = sep;
	*s++ = ']';
	*s = '\0';
	return outstr;
}

#define sprintsigmask_val(prefix, mask) \
	sprintsigmask_n((prefix), &(mask), sizeof(mask))

#define tprintsigmask_val(prefix, mask) \
	tprints(sprintsigmask_n((prefix), &(mask), sizeof(mask)))

void
printsignal(int nr)
{
	tprints(signame(nr));
}

void
print_sigset_addr_len(struct tcb *tcp, long addr, long len)
{
	char mask[NSIG / 8];

	if (!addr) {
		tprints("NULL");
		return;
	}
	/* Here len is usually equals NSIG / 8 or current_wordsize.
	 * But we code this defensively:
	 */
	if (len < 0) {
 bad:
		tprintf("%#lx", addr);
		return;
	}
	if (len >= NSIG / 8)
		len = NSIG / 8;
	else
		len = (len + 3) & ~3;

	if (umoven(tcp, addr, len, mask) < 0)
		goto bad;
	tprints(sprintsigmask_n("", mask, len));
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
#define SYS_SECCOMP     1       /* seccomp triggered */
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
#define SI_KERNEL	0x80	/* sent by kernel */
#define SI_USER         0       /* sent by kill, sigsend, raise */
#define SI_QUEUE        -1      /* sent by sigqueue */
#define SI_TIMER        -2      /* sent by timer expiration */
#define SI_MESGQ        -3      /* sent by real time mesq state change */
#define SI_ASYNCIO      -4      /* sent by AIO completion */
#define SI_SIGIO	-5	/* sent by SIGIO */
#define SI_TKILL	-6	/* sent by tkill */
#define SI_DETHREAD	-7	/* sent by execve killing subsidiary threads */
#define SI_ASYNCNL	-60     /* sent by asynch name lookup completion */
#endif

#ifndef SI_FROMUSER
# define SI_FROMUSER(sip)	((sip)->si_code <= 0)
#endif

#include "xlat/siginfo_codes.h"
#include "xlat/sigill_codes.h"
#include "xlat/sigfpe_codes.h"
#include "xlat/sigtrap_codes.h"
#include "xlat/sigchld_codes.h"
#include "xlat/sigpoll_codes.h"
#include "xlat/sigprof_codes.h"

#ifdef SIGEMT
#include "xlat/sigemt_codes.h"
#endif

#include "xlat/sigsegv_codes.h"
#include "xlat/sigbus_codes.h"

#ifndef SYS_SECCOMP
# define SYS_SECCOMP 1
#endif
#include "xlat/sigsys_codes.h"

static void
printsigsource(const siginfo_t *sip)
{
	tprintf(", si_pid=%lu, si_uid=%lu",
		(unsigned long) sip->si_pid,
		(unsigned long) sip->si_uid);
}

static void
printsigval(const siginfo_t *sip, int verbose)
{
	if (!verbose)
		tprints(", ...");
	else
		tprintf(", si_value={int=%u, ptr=%#lx}",
			sip->si_int,
			(unsigned long) sip->si_ptr);
}

void
printsiginfo(const siginfo_t *sip, int verbose)
{
	const char *code;

	if (sip->si_signo == 0) {
		tprints("{}");
		return;
	}
	tprints("{si_signo=");
	printsignal(sip->si_signo);
	code = xlookup(siginfo_codes, sip->si_code);
	if (!code) {
		switch (sip->si_signo) {
		case SIGTRAP:
			code = xlookup(sigtrap_codes, sip->si_code);
			break;
		case SIGCHLD:
			code = xlookup(sigchld_codes, sip->si_code);
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
#ifdef SIGEMT
		case SIGEMT:
			code = xlookup(sigemt_codes, sip->si_code);
			break;
#endif
		case SIGFPE:
			code = xlookup(sigfpe_codes, sip->si_code);
			break;
		case SIGSEGV:
			code = xlookup(sigsegv_codes, sip->si_code);
			break;
		case SIGBUS:
			code = xlookup(sigbus_codes, sip->si_code);
			break;
		case SIGSYS:
			code = xlookup(sigsys_codes, sip->si_code);
			break;
		}
	}
	if (code)
		tprintf(", si_code=%s", code);
	else
		tprintf(", si_code=%#x", sip->si_code);
#ifdef SI_NOINFO
	if (sip->si_code != SI_NOINFO)
#endif
	{
		if (sip->si_errno) {
			tprints(", si_errno=");
			if ((unsigned) sip->si_errno < nerrnos
			    && errnoent[sip->si_errno])
				tprints(errnoent[sip->si_errno]);
			else
				tprintf("%d", sip->si_errno);
		}
#ifdef SI_FROMUSER
		if (SI_FROMUSER(sip)) {
			switch (sip->si_code) {
#ifdef SI_USER
			case SI_USER:
				printsigsource(sip);
				break;
#endif
#ifdef SI_TKILL
			case SI_TKILL:
				printsigsource(sip);
				break;
#endif
#if defined SI_TIMER \
 && defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
			case SI_TIMER:
				tprintf(", si_timerid=%#x, si_overrun=%d",
					sip->si_timerid, sip->si_overrun);
				printsigval(sip, verbose);
				break;
#endif
			default:
				printsigsource(sip);
				if (sip->si_ptr)
					printsigval(sip, verbose);
				break;
			}
		}
		else
#endif /* SI_FROMUSER */
		{
			switch (sip->si_signo) {
			case SIGCHLD:
				printsigsource(sip);
				tprints(", si_status=");
				if (sip->si_code == CLD_EXITED)
					tprintf("%d", sip->si_status);
				else
					printsignal(sip->si_status);
				if (!verbose)
					tprints(", ...");
				else
					tprintf(", si_utime=%llu, si_stime=%llu",
						(unsigned long long) sip->si_utime,
						(unsigned long long) sip->si_stime);
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
#ifdef HAVE_SIGINFO_T_SI_SYSCALL
			case SIGSYS:
				tprintf(", si_call_addr=%#lx, si_syscall=%d, si_arch=%u",
					(unsigned long) sip->si_call_addr,
					sip->si_syscall, sip->si_arch);
				break;
#endif
			default:
				if (sip->si_pid || sip->si_uid)
				        printsigsource(sip);
				if (sip->si_ptr)
					printsigval(sip, verbose);
			}
		}
	}
	tprints("}");
}

void
printsiginfo_at(struct tcb *tcp, long addr)
{
	siginfo_t si;
	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &si) < 0) {
		tprints("{???}");
		return;
	}
	printsiginfo(&si, verbose(tcp));
}

int
sys_sigsetmask(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintsigmask_val("", tcp->u_arg[0]);
	}
	else if (!syserror(tcp)) {
		tcp->auxstr = sprintsigmask_val("old mask ", tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

#ifdef HAVE_SIGACTION

struct old_sigaction {
	/* sa_handler may be a libc #define, need to use other name: */
#ifdef MIPS
	unsigned int sa_flags;
	void (*__sa_handler)(int);
	/* Kernel treats sa_mask as an array of longs. */
	unsigned long sa_mask[NSIG / sizeof(long) ? NSIG / sizeof(long) : 1];
#else
	void (*__sa_handler)(int);
	unsigned long sa_mask;
	unsigned long sa_flags;
#endif /* !MIPS */
#if HAVE_SA_RESTORER
	void (*sa_restorer)(void);
#endif
};

struct old_sigaction32 {
	/* sa_handler may be a libc #define, need to use other name: */
	uint32_t __sa_handler;
	uint32_t sa_mask;
	uint32_t sa_flags;
#if HAVE_SA_RESTORER
	uint32_t sa_restorer;
#endif
};

static void
decode_old_sigaction(struct tcb *tcp, long addr)
{
	struct old_sigaction sa;
	int r;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp))) {
		tprintf("%#lx", addr);
		return;
	}

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize != sizeof(sa.__sa_handler) && current_wordsize == 4) {
		struct old_sigaction32 sa32;
		r = umove(tcp, addr, &sa32);
		if (r >= 0) {
			memset(&sa, 0, sizeof(sa));
			sa.__sa_handler = (void*)(uintptr_t)sa32.__sa_handler;
			sa.sa_flags = sa32.sa_flags;
#if HAVE_SA_RESTORER && defined SA_RESTORER
			sa.sa_restorer = (void*)(uintptr_t)sa32.sa_restorer;
#endif
			sa.sa_mask = sa32.sa_mask;
		}
	} else
#endif
	{
		r = umove(tcp, addr, &sa);
	}
	if (r < 0) {
		tprints("{...}");
		return;
	}

	/* Architectures using function pointers, like
	 * hppa, may need to manipulate the function pointer
	 * to compute the result of a comparison. However,
	 * the __sa_handler function pointer exists only in
	 * the address space of the traced process, and can't
	 * be manipulated by strace. In order to prevent the
	 * compiler from generating code to manipulate
	 * __sa_handler we cast the function pointers to long. */
	if ((long)sa.__sa_handler == (long)SIG_ERR)
		tprints("{SIG_ERR, ");
	else if ((long)sa.__sa_handler == (long)SIG_DFL)
		tprints("{SIG_DFL, ");
	else if ((long)sa.__sa_handler == (long)SIG_IGN)
		tprints("{SIG_IGN, ");
	else
		tprintf("{%#lx, ", (long) sa.__sa_handler);
#ifdef MIPS
	tprintsigmask_addr("", sa.sa_mask);
#else
	tprintsigmask_val("", sa.sa_mask);
#endif
	tprints(", ");
	printflags(sigact_flags, sa.sa_flags, "SA_???");
#if HAVE_SA_RESTORER && defined SA_RESTORER
	if (sa.sa_flags & SA_RESTORER)
		tprintf(", %p", sa.sa_restorer);
#endif
	tprints("}");
}

int
sys_sigaction(struct tcb *tcp)
{
	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprints(", ");
		decode_old_sigaction(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else
		decode_old_sigaction(tcp, tcp->u_arg[2]);
	return 0;
}

int
sys_signal(struct tcb *tcp)
{
	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprints(", ");
		switch (tcp->u_arg[1]) {
		case (long) SIG_ERR:
			tprints("SIG_ERR");
			break;
		case (long) SIG_DFL:
			tprints("SIG_DFL");
			break;
		case (long) SIG_IGN:
			tprints("SIG_IGN");
			break;
		default:
			tprintf("%#lx", tcp->u_arg[1]);
		}
		return 0;
	}
	else if (!syserror(tcp)) {
		switch (tcp->u_rval) {
		case (long) SIG_ERR:
			tcp->auxstr = "SIG_ERR"; break;
		case (long) SIG_DFL:
			tcp->auxstr = "SIG_DFL"; break;
		case (long) SIG_IGN:
			tcp->auxstr = "SIG_IGN"; break;
		default:
			tcp->auxstr = NULL;
		}
		return RVAL_HEX | RVAL_STR;
	}
	return 0;
}

#endif /* HAVE_SIGACTION */

int
sys_siggetmask(struct tcb *tcp)
{
	if (exiting(tcp)) {
		tcp->auxstr = sprintsigmask_val("mask ", tcp->u_rval);
	}
	return RVAL_HEX | RVAL_STR;
}

int
sys_sigsuspend(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintsigmask_val("", tcp->u_arg[2]);
	}
	return 0;
}

#if !defined SS_ONSTACK
#define SS_ONSTACK      1
#define SS_DISABLE      2
#endif

#include "xlat/sigaltstack_flags.h"

static void
print_stack_t(struct tcb *tcp, unsigned long addr)
{
	stack_t ss;
	int r;

	if (!addr) {
		tprints("NULL");
		return;
	}

#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize != sizeof(ss.ss_sp) && current_wordsize == 4) {
		struct {
			uint32_t ss_sp;
			int32_t ss_flags;
			uint32_t ss_size;
		} ss32;
		r = umove(tcp, addr, &ss32);
		if (r >= 0) {
			memset(&ss, 0, sizeof(ss));
			ss.ss_sp = (void*)(unsigned long) ss32.ss_sp;
			ss.ss_flags = ss32.ss_flags;
			ss.ss_size = (unsigned long) ss32.ss_size;
		}
	} else
#endif
	{
		r = umove(tcp, addr, &ss);
	}
	if (r < 0) {
		tprintf("%#lx", addr);
	} else {
		tprintf("{ss_sp=%#lx, ss_flags=", (unsigned long) ss.ss_sp);
		printflags(sigaltstack_flags, ss.ss_flags, "SS_???");
		tprintf(", ss_size=%lu}", (unsigned long) ss.ss_size);
	}
}

int
sys_sigaltstack(struct tcb *tcp)
{
	if (entering(tcp)) {
		print_stack_t(tcp, tcp->u_arg[0]);
	}
	else {
		tprints(", ");
		print_stack_t(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef HAVE_SIGACTION

/* "Old" sigprocmask, which operates with word-sized signal masks */
int
sys_sigprocmask(struct tcb *tcp)
{
# ifdef ALPHA
	if (entering(tcp)) {
		/*
		 * Alpha/OSF is different: it doesn't pass in two pointers,
		 * but rather passes in the new bitmask as an argument and
		 * then returns the old bitmask.  This "works" because we
		 * only have 64 signals to worry about.  If you want more,
		 * use of the rt_sigprocmask syscall is required.
		 * Alpha:
		 *	old = osf_sigprocmask(how, new);
		 * Everyone else:
		 *	ret = sigprocmask(how, &new, &old, ...);
		 */
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprintsigmask_val(", ", tcp->u_arg[1]);
	}
	else if (!syserror(tcp)) {
		tcp->auxstr = sprintsigmask_val("old mask ", tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	}
# else /* !ALPHA */
	if (entering(tcp)) {
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprints(", ");
		print_sigset_addr_len(tcp, tcp->u_arg[1], current_wordsize);
		tprints(", ");
	}
	else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else
			print_sigset_addr_len(tcp, tcp->u_arg[2], current_wordsize);
	}
# endif /* !ALPHA */
	return 0;
}

#endif /* HAVE_SIGACTION */

int
sys_kill(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %s",
			widen_to_long(tcp->u_arg[0]),
			signame(tcp->u_arg[1])
		);
	}
	return 0;
}

int
sys_tgkill(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%ld, %ld, %s",
			widen_to_long(tcp->u_arg[0]),
			widen_to_long(tcp->u_arg[1]),
			signame(tcp->u_arg[2])
		);
	}
	return 0;
}

int
sys_sigpending(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			print_sigset_addr_len(tcp, tcp->u_arg[0], current_wordsize);
	}
	return 0;
}

int
sys_rt_sigprocmask(struct tcb *tcp)
{
	/* Note: arg[3] is the length of the sigset. Kernel requires NSIG / 8 */
	if (entering(tcp)) {
		printxval(sigprocmaskcmds, tcp->u_arg[0], "SIG_???");
		tprints(", ");
		print_sigset_addr_len(tcp, tcp->u_arg[1], tcp->u_arg[3]);
		tprints(", ");
	}
	else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[2]);
		else
			print_sigset_addr_len(tcp, tcp->u_arg[2], tcp->u_arg[3]);
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}

/* Structure describing the action to be taken when a signal arrives.  */
struct new_sigaction
{
	/* sa_handler may be a libc #define, need to use other name: */
#ifdef MIPS
	unsigned int sa_flags;
	void (*__sa_handler)(int);
#else
	void (*__sa_handler)(int);
	unsigned long sa_flags;
#endif /* !MIPS */
#if HAVE_SA_RESTORER
	void (*sa_restorer)(void);
#endif
	/* Kernel treats sa_mask as an array of longs. */
	unsigned long sa_mask[NSIG / sizeof(long) ? NSIG / sizeof(long) : 1];
};
/* Same for i386-on-x86_64 and similar cases */
struct new_sigaction32
{
	uint32_t __sa_handler;
	uint32_t sa_flags;
#if HAVE_SA_RESTORER
	uint32_t sa_restorer;
#endif
	uint32_t sa_mask[2 * (NSIG / sizeof(long) ? NSIG / sizeof(long) : 1)];
};

static void
decode_new_sigaction(struct tcb *tcp, long addr)
{
	struct new_sigaction sa;
	int r;

	if (!addr) {
		tprints("NULL");
		return;
	}
	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp))) {
		tprintf("%#lx", addr);
		return;
	}
#if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
	if (current_wordsize != sizeof(sa.sa_flags) && current_wordsize == 4) {
		struct new_sigaction32 sa32;
		r = umove(tcp, addr, &sa32);
		if (r >= 0) {
			memset(&sa, 0, sizeof(sa));
			sa.__sa_handler = (void*)(unsigned long)sa32.__sa_handler;
			sa.sa_flags     = sa32.sa_flags;
#if HAVE_SA_RESTORER && defined SA_RESTORER
			sa.sa_restorer  = (void*)(unsigned long)sa32.sa_restorer;
#endif
			/* Kernel treats sa_mask as an array of longs.
			 * For 32-bit process, "long" is uint32_t, thus, for example,
			 * 32th bit in sa_mask will end up as bit 0 in sa_mask[1].
			 * But for (64-bit) kernel, 32th bit in sa_mask is
			 * 32th bit in 0th (64-bit) long!
			 * For little-endian, it's the same.
			 * For big-endian, we swap 32-bit words.
			 */
			sa.sa_mask[0] = sa32.sa_mask[0] + ((long)(sa32.sa_mask[1]) << 32);
		}
	} else
#endif
	{
		r = umove(tcp, addr, &sa);
	}
	if (r < 0) {
		tprints("{...}");
		return;
	}
	/* Architectures using function pointers, like
	 * hppa, may need to manipulate the function pointer
	 * to compute the result of a comparison. However,
	 * the __sa_handler function pointer exists only in
	 * the address space of the traced process, and can't
	 * be manipulated by strace. In order to prevent the
	 * compiler from generating code to manipulate
	 * __sa_handler we cast the function pointers to long. */
	if ((long)sa.__sa_handler == (long)SIG_ERR)
		tprints("{SIG_ERR, ");
	else if ((long)sa.__sa_handler == (long)SIG_DFL)
		tprints("{SIG_DFL, ");
	else if ((long)sa.__sa_handler == (long)SIG_IGN)
		tprints("{SIG_IGN, ");
	else
		tprintf("{%#lx, ", (long) sa.__sa_handler);
	/*
	 * Sigset size is in tcp->u_arg[4] (SPARC)
	 * or in tcp->u_arg[3] (all other),
	 * but kernel won't handle sys_rt_sigaction
	 * with wrong sigset size (just returns EINVAL instead).
	 * We just fetch the right size, which is NSIG / 8.
	 */
	tprintsigmask_val("", sa.sa_mask);
	tprints(", ");

	printflags(sigact_flags, sa.sa_flags, "SA_???");
#if HAVE_SA_RESTORER && defined SA_RESTORER
	if (sa.sa_flags & SA_RESTORER)
		tprintf(", %p", sa.sa_restorer);
#endif
	tprints("}");
}

int
sys_rt_sigaction(struct tcb *tcp)
{
	if (entering(tcp)) {
		printsignal(tcp->u_arg[0]);
		tprints(", ");
		decode_new_sigaction(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		decode_new_sigaction(tcp, tcp->u_arg[2]);
#if defined(SPARC) || defined(SPARC64)
		tprintf(", %#lx, %lu", tcp->u_arg[3], tcp->u_arg[4]);
#elif defined(ALPHA)
		tprintf(", %lu, %#lx", tcp->u_arg[3], tcp->u_arg[4]);
#else
		tprintf(", %lu", tcp->u_arg[3]);
#endif
	}
	return 0;
}

int
sys_rt_sigpending(struct tcb *tcp)
{
	if (exiting(tcp)) {
		/*
		 * One of the few syscalls where sigset size (arg[1])
		 * is allowed to be <= NSIG / 8, not strictly ==.
		 * This allows non-rt sigpending() syscall
		 * to reuse rt_sigpending() code in kernel.
		 */
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			print_sigset_addr_len(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_rt_sigsuspend(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* NB: kernel requires arg[1] == NSIG / 8 */
		print_sigset_addr_len(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

static void
print_sigqueueinfo(struct tcb *tcp, int sig, unsigned long uinfo)
{
	printsignal(sig);
	tprints(", ");
	printsiginfo_at(tcp, uinfo);
}

int
sys_rt_sigqueueinfo(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
		print_sigqueueinfo(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

int
sys_rt_tgsigqueueinfo(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%lu, %lu, ", tcp->u_arg[0], tcp->u_arg[1]);
		print_sigqueueinfo(tcp, tcp->u_arg[2], tcp->u_arg[3]);
	}
	return 0;
}

int sys_rt_sigtimedwait(struct tcb *tcp)
{
	/* NB: kernel requires arg[3] == NSIG / 8 */
	if (entering(tcp)) {
		print_sigset_addr_len(tcp, tcp->u_arg[0], tcp->u_arg[3]);
		tprints(", ");
		/* This is the only "return" parameter, */
		if (tcp->u_arg[1] != 0)
			return 0;
		/* ... if it's NULL, can decode all on entry */
		tprints("NULL, ");
	}
	else if (tcp->u_arg[1] != 0) {
		/* syscall exit, and u_arg[1] wasn't NULL */
		printsiginfo_at(tcp, tcp->u_arg[1]);
		tprints(", ");
	}
	else {
		/* syscall exit, and u_arg[1] was NULL */
		return 0;
	}
	print_timespec(tcp, tcp->u_arg[2]);
	tprintf(", %lu", tcp->u_arg[3]);
	return 0;
};

int
sys_restart_syscall(struct tcb *tcp)
{
	if (entering(tcp))
		tprints("<... resuming interrupted call ...>");
	return 0;
}

static int
do_signalfd(struct tcb *tcp, int flags_arg)
{
	/* NB: kernel requires arg[2] == NSIG / 8 */
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		print_sigset_addr_len(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprintf(", %lu", tcp->u_arg[2]);
		if (flags_arg >= 0) {
			tprints(", ");
			printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
		}
	}
	return 0;
}

int
sys_signalfd(struct tcb *tcp)
{
	return do_signalfd(tcp, -1);
}

int
sys_signalfd4(struct tcb *tcp)
{
	return do_signalfd(tcp, 3);
}
