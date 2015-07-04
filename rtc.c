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
print_rtc(struct tcb *tcp, const struct rtc_time *rt)
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

int
rtc_ioctl(struct tcb *tcp, const unsigned int code, const long arg)
{
	switch (code) {
	case RTC_ALM_SET:
	case RTC_SET_TIME:
		if (entering(tcp)) {
			struct rtc_time rt;
			if (umove(tcp, arg, &rt) < 0)
				tprintf(", %#lx", arg);
			else {
				tprints(", ");
				print_rtc(tcp, &rt);
			}
		}
		break;
	case RTC_ALM_READ:
	case RTC_RD_TIME:
		if (exiting(tcp)) {
			struct rtc_time rt;
			if (syserror(tcp) || umove(tcp, arg, &rt) < 0)
				tprintf(", %#lx", arg);
			else {
				tprints(", ");
				print_rtc(tcp, &rt);
			}
		}
		break;
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
		if (entering(tcp))
			tprintf(", %lu", arg);
		break;
	case RTC_IRQP_READ:
	case RTC_EPOCH_READ:
		if (exiting(tcp))
			tprintf(", %lu", arg);
		break;
	case RTC_WKALM_SET:
		if (entering(tcp)) {
			struct rtc_wkalrm wk;
			if (umove(tcp, arg, &wk) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", {enabled=%d, pending=%d, ",
					wk.enabled, wk.pending);
				print_rtc(tcp, &wk.time);
				tprints("}");
			}
		}
		break;
	case RTC_WKALM_RD:
		if (exiting(tcp)) {
			struct rtc_wkalrm wk;
			if (syserror(tcp) || umove(tcp, arg, &wk) < 0)
				tprintf(", %#lx", arg);
			else {
				tprintf(", {enabled=%d, pending=%d, ",
					wk.enabled, wk.pending);
				print_rtc(tcp, &wk.time);
				tprints("}");
			}
		}
		break;
	default:
		return 0;
	}
	return 1;
}
