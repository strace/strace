/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(kernel_rusage_t)

#include "kernel_rusage.h"

#include MPERS_DEFS

#include <sys/resource.h>

MPERS_PRINTER_DECL(void, printrusage,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	kernel_rusage_t ru;

	if (umove_or_printaddr(tcp, addr, &ru))
		return;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(ru, ru_utime,
			    MPERS_FUNC_NAME(print_struct_timeval));
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(ru, ru_stime,
			    MPERS_FUNC_NAME(print_struct_timeval));
	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_maxrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_ixrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_idrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_isrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_minflt);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_majflt);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nswap);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_inblock);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_oublock);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_msgsnd);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_msgrcv);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nsignals);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nvcsw);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nivcsw);
	}
	tprint_struct_end();
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

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(ru, ru_utime, print_timeval32_t);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(ru, ru_stime, print_timeval32_t);
	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_maxrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_ixrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_idrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_isrss);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_minflt);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_majflt);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nswap);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_inblock);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_oublock);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_msgsnd);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_msgrcv);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nsignals);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nvcsw);
		tprint_struct_next();
		PRINT_FIELD_U(ru, ru_nivcsw);
	}
	tprint_struct_end();
}
#endif
