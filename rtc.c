/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Dmitry V. Levin <ldv@altlinux.org>
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
#include <linux/ioctl.h>
#include <linux/rtc.h>

static void
print_rtc_time(struct tcb *tcp, const struct rtc_time *rt)
{
	tprintf("{tm_sec=%d, tm_min=%d, tm_hour=%d, "
		"tm_mday=%d, tm_mon=%d, tm_year=%d, ",
		rt->tm_sec, rt->tm_min, rt->tm_hour,
		rt->tm_mday, rt->tm_mon, rt->tm_year);
	if (!abbrev(tcp))
		tprintf("tm_wday=%d, tm_yday=%d, tm_isdst=%d}",
			rt->tm_wday, rt->tm_yday, rt->tm_isdst);
	else
		tprints("...}");
}

static void
decode_rtc_time(struct tcb *tcp, const long addr)
{
	struct rtc_time rt;

	tprints(", ");
	if (!umove_or_printaddr(tcp, addr, &rt))
		print_rtc_time(tcp, &rt);
}

static void
decode_rtc_wkalrm(struct tcb *tcp, const long addr)
{
	struct rtc_wkalrm wk;

	tprints(", ");
	if (!umove_or_printaddr(tcp, addr, &wk)) {
		tprintf("{enabled=%d, pending=%d, ", wk.enabled, wk.pending);
		print_rtc_time(tcp, &wk.time);
		tprints("}");
	}
}

int
rtc_ioctl(struct tcb *tcp, const unsigned int code, const long arg)
{
	switch (code) {
	case RTC_ALM_SET:
	case RTC_SET_TIME:
		decode_rtc_time(tcp, arg);
		break;
	case RTC_ALM_READ:
	case RTC_RD_TIME:
		if (entering(tcp))
			return 0;
		decode_rtc_time(tcp, arg);
		break;
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
		tprintf(", %lu", arg);
		break;
	case RTC_IRQP_READ:
	case RTC_EPOCH_READ:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_ulong(tcp, arg);
		break;
	case RTC_WKALM_SET:
		decode_rtc_wkalrm(tcp, arg);
		break;
	case RTC_WKALM_RD:
		if (entering(tcp))
			return 0;
		decode_rtc_wkalrm(tcp, arg);
		break;
#ifdef RTC_VL_READ
	case RTC_VL_READ:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;
#endif
	default:
		return RVAL_DECODED;
	}

	return RVAL_DECODED | 1;
}
