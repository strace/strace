/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2001 John Hughes <john@Calva.COM>
 * Copyright (c) 2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(siginfo_t)

#include <signal.h>
#include <linux/audit.h>

#include MPERS_DEFS

#ifndef IN_MPERS
# include "printsiginfo.h"
#endif

#define XLAT_MACROS_ONLY
/* For xlat/audit_arch.h */
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
# include "xlat/alpha_trap_codes.h"
#endif

#ifdef SIGEMT
# include "xlat/sigemt_codes.h"
#endif

#ifdef HAVE_SIGINFO_T_SI_PERF_FLAGS
# include "xlat/sigtrap_perf_flags.h"
#endif

#ifndef SI_FROMUSER
# define SI_FROMUSER(sip)	((sip)->si_code <= 0)
#endif

static void
printsigsource(struct tcb *tcp, const siginfo_t *sip)
{
	tprint_struct_next();
	PRINT_FIELD_TGID(*sip, si_pid, tcp);
	tprint_struct_next();
	PRINT_FIELD_ID(*sip, si_uid);
}

static void
printsigval(const siginfo_t *sip)
{
	tprint_struct_next();
	PRINT_FIELD_D(*sip, si_int);
	tprint_struct_next();
	PRINT_FIELD_PTR(*sip, si_ptr);
}

static void
print_si_code(const unsigned int si_code, const int si_signo)
{
	static const struct xlat * const si_codes[] = {
		[SIGTRAP] = sigtrap_codes,
		[SIGCHLD] = sigchld_codes,
		[SIGIO]   = sigpoll_codes, /* SIGPOLL */
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
print_si_info(struct tcb *tcp, const siginfo_t *sip)
{
	if (sip->si_errno) {
		tprint_struct_next();
		PRINT_FIELD_ERR_U(*sip, si_errno);
	}

	if (SI_FROMUSER(sip)) {
		switch (sip->si_code) {
		case SI_USER:
			printsigsource(tcp, sip);
			break;
		case SI_TKILL:
			printsigsource(tcp, sip);
			break;
#if defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
		case SI_TIMER:
			tprint_struct_next();
			PRINT_FIELD_X(*sip, si_timerid);
			tprint_struct_next();
			PRINT_FIELD_D(*sip, si_overrun);
			printsigval(sip);
			break;
#endif
		case SI_SIGIO:
			tprint_struct_next();
			PRINT_FIELD_D(*sip, si_band);
			tprint_struct_next();
			PRINT_FIELD_FD(*sip, si_fd, tcp);
			break;
		default:
			printsigsource(tcp, sip);
			if (sip->si_ptr)
				printsigval(sip);
			break;
		}
	} else {
		switch (sip->si_signo) {
		case SIGCHLD:
			printsigsource(tcp, sip);
			if (sip->si_code == CLD_EXITED) {
				tprint_struct_next();
				PRINT_FIELD_D(*sip, si_status);
			} else {
				tprint_struct_next();
				PRINT_FIELD_OBJ_VAL(*sip, si_status, printsignal);
			}
			tprint_struct_next();
			PRINT_FIELD_CLOCK_T(*sip, si_utime);
			tprint_struct_next();
			PRINT_FIELD_CLOCK_T(*sip, si_stime);
			break;
		case SIGILL:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
#if defined(SPARC) || defined(SPARC64)
			tprint_struct_next();
			PRINT_FIELD_D(*sip, si_trapno);
#endif /* SPARC || SPARC64 */
			break;
		case SIGFPE:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
#if defined ALPHA && defined HAVE_SIGINFO_T_SI_TRAPNO
			tprint_struct_next();
			PRINT_FIELD_XVAL_D(*sip, si_trapno, alpha_trap_codes,
					   "GEN_???");
#endif /* ALPHA */
			break;
		case SIGBUS:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
#if !defined(BUS_OPFETCH) && defined(HAVE_SIGINFO_T_SI_ADDR_LSB)
			switch (sip->si_code) {
			case BUS_MCEERR_AR:
			case BUS_MCEERR_AO:
				tprint_struct_next();
				PRINT_FIELD_X(*sip, si_addr_lsb);
				break;
			}
#endif /* !BUS_OPFETCH && HAVE_SIGINFO_T_SI_ADDR_LSB */
			break;
		case SIGSEGV:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
#if (!defined(SEGV_STACKFLOW) && defined(HAVE_SIGINFO_T_SI_LOWER)) \
    || (!defined(__SEGV_PSTKOVF) && defined(HAVE_SIGINFO_T_SI_PKEY))
			switch (sip->si_code) {
# if !defined(SEGV_STACKFLOW) && defined(HAVE_SIGINFO_T_SI_LOWER)
			case SEGV_BNDERR:
				tprint_struct_next();
				PRINT_FIELD_PTR(*sip, si_lower);
				tprint_struct_next();
				PRINT_FIELD_PTR(*sip, si_upper);
				break;
# endif /* !SEGV_STACKFLOW && HAVE_SIGINFO_T_SI_LOWER */
# if !defined(__SEGV_PSTKOVF) && defined(HAVE_SIGINFO_T_SI_PKEY)
			case SEGV_PKUERR:
				tprint_struct_next();
				PRINT_FIELD_U(*sip, si_pkey);
				break;
# endif /* !__SEGV_PSTKOVF && HAVE_SIGINFO_T_SI_PKEY */
			}
#endif /* !SEGV_STACKFLOW && HAVE_SIGINFO_T_SI_LOWER
	* || !__SEGV_PSTKOVF && HAVE_SIGINFO_T_SI_PKEY */
			break;
		case SIGTRAP:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
#if (defined ALPHA && defined HAVE_SIGINFO_T_SI_TRAPNO) \
 || defined HAVE_SIGINFO_T_SI_PERF_DATA
			switch (sip->si_code) {
# if defined ALPHA && defined HAVE_SIGINFO_T_SI_TRAPNO
			case TRAP_UNK:
				tprint_struct_next();
				PRINT_FIELD_XVAL_D(*sip, si_trapno,
						   alpha_trap_codes, "GEN_???");
				break;
# endif /* ALPHA && HAVE_SIGINFO_T_SI_TRAPNO */
# ifdef HAVE_SIGINFO_T_SI_PERF_DATA
			case TRAP_PERF:
				tprint_struct_next();
				PRINT_FIELD_X(*sip, si_perf_data);
#  ifdef HAVE_SIGINFO_T_SI_PERF_TYPE
				tprint_struct_next();
				PRINT_FIELD_XVAL(*sip, si_perf_type,
						 perf_type_id, "PERF_TYPE_???");
#  endif /* HAVE_SIGINFO_T_SI_PERF_TYPE */
#  ifdef HAVE_SIGINFO_T_SI_PERF_FLAGS
				tprint_struct_next();
				PRINT_FIELD_FLAGS(*sip, si_perf_flags,
						  sigtrap_perf_flags,
						  "TRAP_PERF_FLAG_???");
#  endif /* HAVE_SIGINFO_T_SI_PERF_FLAGS */
# endif /* HAVE_SIGINFO_T_SI_PERF_DATA */
			}
#endif /* ALPHA || HAVE_SIGINFO_T_SI_PERF_DATA */
			break;
#ifdef SIGEMT
		case SIGEMT:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
			break;
#endif
		case SIGIO: /* SIGPOLL */
			switch (sip->si_code) {
			case POLL_IN:  case POLL_OUT: case POLL_MSG:
			case POLL_ERR: case POLL_PRI: case POLL_HUP:
				tprint_struct_next();
				PRINT_FIELD_D(*sip, si_band);
				tprint_struct_next();
				PRINT_FIELD_FD(*sip, si_fd, tcp);
				break;
			}
			break;
#ifdef HAVE_SIGINFO_T_SI_SYSCALL
		case SIGSYS:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_call_addr);
			tprint_struct_next();
			PRINT_FIELD_SYSCALL_NAME(*sip, si_syscall,
						 sip->si_arch);
			tprint_struct_next();
			PRINT_FIELD_XVAL(*sip, si_arch, audit_arch,
					 "AUDIT_ARCH_???");
			break;
#endif
		default:
			if (sip->si_pid || sip->si_uid)
				printsigsource(tcp, sip);
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
	tprint_struct_begin();

	if (sip->si_signo) {
		PRINT_FIELD_OBJ_VAL(*sip, si_signo, printsignal);
		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(*sip, si_code, print_si_code,
				    sip->si_signo);

#ifdef SI_NOINFO
		if (sip->si_code != SI_NOINFO)
#endif
			print_si_info(tcp, sip);
	}

	tprint_struct_end();
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
