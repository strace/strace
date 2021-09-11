/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2001 John Hughes <john@Calva.COM>
 * Copyright (c) 2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2021 The strace developers.
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

#ifdef SIGEMT
# include "xlat/sigemt_codes.h"
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
	const char *code = xlookup(siginfo_codes, si_code);

	if (!code) {
		switch (si_signo) {
		case SIGTRAP:
			code = xlookup(sigtrap_codes, si_code);
			break;
		case SIGCHLD:
			code = xlookup(sigchld_codes, si_code);
			break;
		case SIGPOLL:
			code = xlookup(sigpoll_codes, si_code);
			break;
		case SIGPROF:
			code = xlookup(sigprof_codes, si_code);
			break;
		case SIGILL:
			code = xlookup(sigill_codes, si_code);
			break;
#ifdef SIGEMT
		case SIGEMT:
			code = xlookup(sigemt_codes, si_code);
			break;
#endif
		case SIGFPE:
			code = xlookup(sigfpe_codes, si_code);
			break;
		case SIGSEGV:
			code = xlookup(sigsegv_codes, si_code);
			break;
		case SIGBUS:
			code = xlookup(sigbus_codes, si_code);
			break;
		case SIGSYS:
			code = xlookup(sigsys_codes, si_code);
			break;
		}
	}

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
			PRINT_FIELD_U(*sip, si_utime);
			tprint_struct_next();
			PRINT_FIELD_U(*sip, si_stime);
			break;
		case SIGILL: case SIGFPE:
		case SIGSEGV: case SIGBUS:
			tprint_struct_next();
			PRINT_FIELD_PTR(*sip, si_addr);
			break;
		case SIGPOLL:
			switch (sip->si_code) {
			case POLL_IN: case POLL_OUT: case POLL_MSG:
				tprint_struct_next();
				PRINT_FIELD_D(*sip, si_band);
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
