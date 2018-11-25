/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2001 John Hughes <john@Calva.COM>
 * Copyright (c) 2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2018 The strace developers.
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

#include DEF_MPERS_TYPE(siginfo_t)

#include <signal.h>
#include <linux/audit.h>

#include MPERS_DEFS

#include "nr_prefix.c"

#ifndef IN_MPERS
#include "printsiginfo.h"
#endif

#define XLAT_MACROS_ONLY
/* For xlat/audit_arch.h */
# include "xlat/audit_arch_flags.h"
# include "xlat/elf_em.h"
#undef XLAT_MACROS_ONLY

#include "xlat/audit_arch.h"
#include "xlat/sigbus_codes.h"
#include "xlat/sigchld_codes.h"
#include "xlat/sigfpe_codes.h"
#include "xlat/sigill_codes.h"
#include "xlat/siginfo_codes.h"
#include "xlat/sigpoll_codes.h"
#include "xlat/sigprof_codes.h"
#include "xlat/sigsegv_codes.h"
#include "xlat/sigsys_codes.h"
#include "xlat/sigtrap_codes.h"

#ifdef ALPHA
# include <asm/gentrap.h>
# include "xlat/alpha_gentrap.h"
#endif /* ALPHA */

#ifdef SIGEMT
# include "xlat/sigemt_codes.h"
#endif

#ifndef SI_FROMUSER
# define SI_FROMUSER(sip)	((sip)->si_code <= 0)
#endif

static void
printsigsource(const siginfo_t *sip)
{
	tprintf(", si_pid=%u", (unsigned int) sip->si_pid);
	printuid(", si_uid=", (unsigned int) sip->si_uid);
}

static void
printsigval(const siginfo_t *sip)
{
	tprintf(", si_value={int=%d, ptr=", sip->si_int);
	printaddr(ptr_to_kulong(sip->si_ptr));
	tprints("}");
}

static void
print_si_code(int si_signo, unsigned int si_code)
{
	static const struct xlat * const si_codes[] = {
		[SIGTRAP] = sigtrap_codes,
		[SIGCHLD] = sigchld_codes,
		[SIGPOLL] = sigpoll_codes,
		[SIGPROF] = sigprof_codes,
		[SIGILL]  = sigill_codes,
#ifdef SIGEMT
		[SIGEMT]  = sigemt_codes,
#endif
		[SIGFPE]  = sigfpe_codes,
		[SIGSEGV] = sigsegv_codes,
		[SIGBUS]  = sigbus_codes,
		[SIGSYS]  = sigsys_codes,
	};

	const char *code = xlookup(siginfo_codes, si_code);

	if (!code && (unsigned int) si_signo < ARRAY_SIZE(si_codes)
	    && si_codes[si_signo])
		code = xlookup(si_codes[si_signo], si_code);

	print_xlat_ex(si_code, code, XLAT_STYLE_DEFAULT);
}

static void
print_scconst(unsigned scno)
{
	/*
	 * Note that we can safely use the personality set in
	 * current_personality  here (and don't have to guess it
	 * based on X32_SYSCALL_BIT and si_arch, for example):
	 *  - The signal is delivered as a result of seccomp
	 *    filtering to the process executing forbidden
	 *    syscall.
	 *  - We have set the personality for the tracee during
	 *    the syscall entering.
	 *  - The current_personality is reliably switched in
	 *    the next_event routine, it is set to the
	 *    personality of the last call made (the one that
	 *    triggered the signal delivery).
	 *  - Looks like there are no other cases where SIGSYS
	 *    is delivered from the kernel so far.
	 */
	const char *scname = syscall_name(shuffle_scno(scno));

	if (!scname || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%u", scno);
	if (!scname || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;
	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_ABBREV
		? tprintf : tprintf_comment)("%s%s",
					     nr_prefix(scno), scname);
}

static void
print_fasync_info(struct tcb *tcp, const siginfo_t *sip)
{
	tprintf(", si_band=");
	if ((long) sip->si_band != -1L)
		printflags(pollflags, sip->si_band, "POLL???");
	else
		tprints("-1");

	tprints(", si_fd=");
	printfd(tcp, sip->si_fd);
}

static void
print_si_info(struct tcb *tcp, const siginfo_t *sip)
{
	if (sip->si_errno) {
		tprints(", si_errno=");
		print_err(sip->si_errno, false);
	}

	if (SI_FROMUSER(sip)) {
		switch (sip->si_code) {
		case SI_USER:
			printsigsource(sip);
			break;
		case SI_TKILL:
			printsigsource(sip);
			break;
#if defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
		case SI_TIMER:
			tprintf(", si_timerid=%#x, si_overrun=%d",
				sip->si_timerid, sip->si_overrun);
			printsigval(sip);
			break;
#endif
		case SI_SIGIO:
			print_fasync_info(tcp, sip);
			break;
		default:
			printsigsource(sip);
			if (sip->si_ptr)
				printsigval(sip);
			break;
		}
	} else {
		switch (sip->si_signo) {
		case SIGCHLD:
			printsigsource(sip);
			tprints(", si_status=");
			if (sip->si_code == CLD_EXITED)
				tprintf("%d", sip->si_status);
			else
				printsignal(sip->si_status);
			tprintf(", si_utime=%llu, si_stime=%llu",
				zero_extend_signed_to_ull(sip->si_utime),
				zero_extend_signed_to_ull(sip->si_stime));
			break;
		case SIGILL: case SIGFPE:
		case SIGSEGV: case SIGBUS:
		case SIGTRAP:
#ifdef SIGEMT
		case SIGEMT:
#endif
			tprints(", si_addr=");
			printaddr(ptr_to_kulong(sip->si_addr));
#ifdef HAVE_SIGINFO_T_SI_TRAPNO
# if defined(SPARC)
			if ((sip->si_signo == SIGILL && sip->si_code == ILLTRP)
			    || sip->si_trapno)
# elif defined(ALPHA)
			/*
			 * See arch/alpha/kernel/traps.c:do_entIF, gentrap
			 * handling, and arch/alpha/include/uapi/asm/gentrap.h
			 */
			if (sip->si_trapno
			    || sip->si_signo == SIGFPE
			    || (sip->si_signo == SIGTRAP
				&& sip->si_code == TRAP_UNK))
# endif
			{
				tprints(", si_trapno=");
# ifdef ALPHA
				printxval_d(alpha_gentraps, sip->si_trapno,
					    sip->si_trapno ? "GEN_???" : NULL);
# else /* !ALPHA */
				tprintf("%d")
# endif ALPHA
			}
#endif /* HAVE_SIGINFO_T_SI_TRAPNO */

			break;
		case SIGPOLL:
			switch (sip->si_code) {
			case POLL_IN:  case POLL_OUT: case POLL_MSG:
			case POLL_ERR: case POLL_PRI: case POLL_HUP:
				print_fasync_info(tcp, sip);
			}
			break;
#ifdef HAVE_SIGINFO_T_SI_SYSCALL
		case SIGSYS: {
			tprints(", si_call_addr=");
			printaddr(ptr_to_kulong(sip->si_call_addr));
			tprints(", si_syscall=");
			print_scconst(sip->si_syscall);
			tprints(", si_arch=");
			printxval(audit_arch, sip->si_arch, "AUDIT_ARCH_???");
			break;
		}
#endif
		default:
			if (sip->si_pid || sip->si_uid)
				printsigsource(sip);
			if (sip->si_ptr)
				printsigval(sip);
		}
	}
}

#ifdef IN_MPERS
static
#endif
void
printsiginfo(struct tcb *tcp, const siginfo_t *sip)
{
	if (sip->si_signo == 0) {
		tprints("{}");
		return;
	}
	tprints("{si_signo=");
	printsignal(sip->si_signo);

	tprints(", si_code=");
	print_si_code(sip->si_signo, sip->si_code);

#ifdef SI_NOINFO
	if (sip->si_code != SI_NOINFO)
#endif
		print_si_info(tcp, sip);

	tprints("}");
}

MPERS_PRINTER_DECL(void, printsiginfo_at,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	siginfo_t si;

	if (!umove_or_printaddr(tcp, addr, &si))
		printsiginfo(tcp, &si);
}

static bool
print_siginfo_t(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	printsiginfo(tcp, (const siginfo_t *) elem_buf);
	return true;
}

MPERS_PRINTER_DECL(void, print_siginfo_array, struct tcb *const tcp,
		   const kernel_ulong_t addr, const kernel_ulong_t len)
{
	siginfo_t si;

	print_array(tcp, addr, len, &si, sizeof(si),
		    tfetch_mem, print_siginfo_t, 0);
}
