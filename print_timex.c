/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_timex)

#include <sys/timex.h>
typedef struct timex struct_timex;

#include MPERS_DEFS

#include "xlat/adjtimex_modes.h"
#include "xlat/adjtimex_status.h"

MPERS_PRINTER_DECL(int, print_timex,
		   struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_timex tx;

	if (umove_or_printaddr(tcp, addr, &tx))
		return -1;

	tprints("{modes=");
	printflags(adjtimex_modes, tx.modes, "ADJ_???");
	tprintf(", offset=%jd, freq=%jd, maxerror=%ju, esterror=%ju, status=",
		(intmax_t) tx.offset, (intmax_t) tx.freq,
		(uintmax_t) tx.maxerror, (uintmax_t) tx.esterror);
	printflags(adjtimex_status, tx.status, "STA_???");
	tprintf(", constant=%jd, precision=%ju, tolerance=%jd, time=",
		(intmax_t) tx.constant, (uintmax_t) tx.precision,
		(intmax_t) tx.tolerance);
	MPERS_FUNC_NAME(print_struct_timeval)(&tx.time);
	tprintf(", tick=%jd, ppsfreq=%jd, jitter=%jd",
		(intmax_t) tx.tick, (intmax_t) tx.ppsfreq, (intmax_t) tx.jitter);
	tprintf(", shift=%d, stabil=%jd, jitcnt=%jd",
		tx.shift, (intmax_t) tx.stabil, (intmax_t) tx.jitcnt);
	tprintf(", calcnt=%jd, errcnt=%jd, stbcnt=%jd",
		(intmax_t) tx.calcnt, (intmax_t) tx.errcnt, (intmax_t) tx.stbcnt);
#ifdef HAVE_STRUCT_TIMEX_TAI
	tprintf(", tai=%d", tx.tai);
#endif
	tprints("}");
	return 0;
}
