/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
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
