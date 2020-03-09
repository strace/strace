/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(kernel_rusage_t)

#include "kernel_rusage.h"

#include MPERS_DEFS

#include <sys/resource.h>
#include "print_fields.h"

MPERS_PRINTER_DECL(void, printrusage,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	kernel_rusage_t ru;

	if (umove_or_printaddr(tcp, addr, &ru))
		return;

	tprints("{ru_utime=");
	MPERS_FUNC_NAME(print_struct_timeval)(&ru.ru_utime);
	tprints(", ru_stime=");
	MPERS_FUNC_NAME(print_struct_timeval)(&ru.ru_stime);
	if (abbrev(tcp)) {
		tprints(", ...");
	} else {
		PRINT_FIELD_U(", ", ru, ru_maxrss);
		PRINT_FIELD_U(", ", ru, ru_ixrss);
		PRINT_FIELD_U(", ", ru, ru_idrss);
		PRINT_FIELD_U(", ", ru, ru_isrss);
		PRINT_FIELD_U(", ", ru, ru_minflt);
		PRINT_FIELD_U(", ", ru, ru_majflt);
		PRINT_FIELD_U(", ", ru, ru_nswap);
		PRINT_FIELD_U(", ", ru, ru_inblock);
		PRINT_FIELD_U(", ", ru, ru_oublock);
		PRINT_FIELD_U(", ", ru, ru_msgsnd);
		PRINT_FIELD_U(", ", ru, ru_msgrcv);
		PRINT_FIELD_U(", ", ru, ru_nsignals);
		PRINT_FIELD_U(", ", ru, ru_nvcsw);
		PRINT_FIELD_U(", ", ru, ru_nivcsw);
	}
	tprints("}");
}

#ifdef ALPHA
void
printrusage32(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rusage32 {
		timeval32_t ru_utime;		/* user time used */
		timeval32_t ru_stime;		/* system time used */
		long	ru_maxrss;		/* maximum resident set size */
		long	ru_ixrss;		/* integral shared memory size */
		long	ru_idrss;		/* integral unshared data size */
		long	ru_isrss;		/* integral unshared stack size */
		long	ru_minflt;		/* page reclaims */
		long	ru_majflt;		/* page faults */
		long	ru_nswap;		/* swaps */
		long	ru_inblock;		/* block input operations */
		long	ru_oublock;		/* block output operations */
		long	ru_msgsnd;		/* messages sent */
		long	ru_msgrcv;		/* messages received */
		long	ru_nsignals;		/* signals received */
		long	ru_nvcsw;		/* voluntary context switches */
		long	ru_nivcsw;		/* involuntary " */
	} ru;

	if (umove_or_printaddr(tcp, addr, &ru))
		return;

	tprints("{ru_utime=");
	print_timeval32_t(&ru.ru_utime);
	tprints(", ru_stime=");
	print_timeval32_t(&ru.ru_stime);
	if (abbrev(tcp)) {
		tprints(", ...");
	} else {
		PRINT_FIELD_U(", ", ru, ru_maxrss);
		PRINT_FIELD_U(", ", ru, ru_ixrss);
		PRINT_FIELD_U(", ", ru, ru_idrss);
		PRINT_FIELD_U(", ", ru, ru_isrss);
		PRINT_FIELD_U(", ", ru, ru_minflt);
		PRINT_FIELD_U(", ", ru, ru_majflt);
		PRINT_FIELD_U(", ", ru, ru_nswap);
		PRINT_FIELD_U(", ", ru, ru_inblock);
		PRINT_FIELD_U(", ", ru, ru_oublock);
		PRINT_FIELD_U(", ", ru, ru_msgsnd);
		PRINT_FIELD_U(", ", ru, ru_msgrcv);
		PRINT_FIELD_U(", ", ru, ru_nsignals);
		PRINT_FIELD_U(", ", ru, ru_nvcsw);
		PRINT_FIELD_U(", ", ru, ru_nivcsw);
	}
	tprints("}");
}
#endif
