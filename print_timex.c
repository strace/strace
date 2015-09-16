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
#include <sys/timex.h>

#include "xlat/adjtimex_modes.h"
#include "xlat/adjtimex_status.h"

#if SUPPORTED_PERSONALITIES > 1

# if defined X86_64 || defined X32
#  define current_time_t_is_compat (current_personality == 1)
# else
#  define current_time_t_is_compat (current_wordsize == 4)
# endif

struct timeval32
{
	u_int32_t tv_sec, tv_usec;
};

static void
tprint_timeval32(struct tcb *tcp, const struct timeval32 *tv)
{
	tprintf("{%u, %u}", tv->tv_sec, tv->tv_usec);
}

static int
tprint_timex32(struct tcb *tcp, long addr)
{
	struct {
		unsigned int modes;
		int     offset;
		int     freq;
		int     maxerror;
		int     esterror;
		int     status;
		int     constant;
		int     precision;
		int     tolerance;
		struct timeval32 time;
		int     tick;
		int     ppsfreq;
		int     jitter;
		int     shift;
		int     stabil;
		int     jitcnt;
		int     calcnt;
		int     errcnt;
		int     stbcnt;
	} tx;

	if (umove_or_printaddr(tcp, addr, &tx))
		return -1;

	tprints("{modes=");
	printflags(adjtimex_modes, tx.modes, "ADJ_???");
	tprintf(", offset=%d, freq=%d, maxerror=%d, ",
		tx.offset, tx.freq, tx.maxerror);
	tprintf("esterror=%u, status=", tx.esterror);
	printflags(adjtimex_status, tx.status, "STA_???");
	tprintf(", constant=%d, precision=%u, ",
		tx.constant, tx.precision);
	tprintf("tolerance=%d, time=", tx.tolerance);
	tprint_timeval32(tcp, &tx.time);
	tprintf(", tick=%d, ppsfreq=%d, jitter=%d",
		tx.tick, tx.ppsfreq, tx.jitter);
	tprintf(", shift=%d, stabil=%d, jitcnt=%d",
		tx.shift, tx.stabil, tx.jitcnt);
	tprintf(", calcnt=%d, errcnt=%d, stbcnt=%d",
		tx.calcnt, tx.errcnt, tx.stbcnt);
	tprints("}");
	return 0;
}
#endif /* SUPPORTED_PERSONALITIES > 1 */

static void
tprint_timeval(struct tcb *tcp, const struct timeval *tv)
{
	tprintf("{%ju, %ju}", (uintmax_t) tv->tv_sec, (uintmax_t) tv->tv_usec);
}

int
tprint_timex(struct tcb *tcp, long addr)
{
	struct timex tx;

#if SUPPORTED_PERSONALITIES > 1
	if (current_time_t_is_compat)
		return tprint_timex32(tcp, addr);
#endif
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
	tprint_timeval(tcp, &tx.time);
	tprintf(", tick=%jd, ppsfreq=%jd, jitter=%jd",
		(intmax_t) tx.tick, (intmax_t) tx.ppsfreq, (intmax_t) tx.jitter);
	tprintf(", shift=%d, stabil=%jd, jitcnt=%jd",
		tx.shift, (intmax_t) tx.stabil, (intmax_t) tx.jitcnt);
	tprintf(", calcnt=%jd, errcnt=%jd, stbcnt=%jd",
		(intmax_t) tx.calcnt, (intmax_t) tx.errcnt, (intmax_t) tx.stbcnt);
	tprints("}");
	return 0;
}
