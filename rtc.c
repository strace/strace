/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_rtc_pll_info)

#include <linux/ioctl.h>
#include <linux/rtc.h>

typedef struct rtc_pll_info struct_rtc_pll_info;

#include MPERS_DEFS

#include "xlat/rtc_vl_flags.h"

#define XLAT_MACROS_ONLY
# include "xlat/rtc_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

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
decode_rtc_time(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rtc_time rt;

	if (!umove_or_printaddr(tcp, addr, &rt))
		print_rtc_time(tcp, &rt);
}

static void
decode_rtc_wkalrm(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct rtc_wkalrm wk;

	if (!umove_or_printaddr(tcp, addr, &wk)) {
		tprintf("{enabled=%d, pending=%d, time=", wk.enabled, wk.pending);
		print_rtc_time(tcp, &wk.time);
		tprints("}");
	}
}

static void
decode_rtc_pll_info(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_rtc_pll_info pll;

	if (!umove_or_printaddr(tcp, addr, &pll))
		tprintf("{pll_ctrl=%d, pll_value=%d, pll_max=%d, pll_min=%d"
			", pll_posmult=%d, pll_negmult=%d, pll_clock=%ld}",
			pll.pll_ctrl, pll.pll_value, pll.pll_max, pll.pll_min,
			pll.pll_posmult, pll.pll_negmult, (long) pll.pll_clock);
}

static void
decode_rtc_vl(struct tcb *const tcp, const kernel_ulong_t addr)
{
	unsigned int val;

	if (umove_or_printaddr(tcp, addr, &val))
		return;

	tprints("[");
	printflags(rtc_vl_flags, val, "RTC_VL_???");
	tprints("]");
}

MPERS_PRINTER_DECL(int, rtc_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case RTC_ALM_READ:
	case RTC_RD_TIME:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case RTC_ALM_SET:
	case RTC_SET_TIME:
		tprints(", ");
		decode_rtc_time(tcp, arg);
		break;
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
		tprintf(", %" PRI_klu, arg);
		break;
	case RTC_IRQP_READ:
	case RTC_EPOCH_READ:
		if (entering(tcp))
			return 0;
		tprints(", ");
		printnum_ulong(tcp, arg);
		break;
	case RTC_WKALM_RD:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case RTC_WKALM_SET:
		tprints(", ");
		decode_rtc_wkalrm(tcp, arg);
		break;
	case RTC_PLL_GET:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case RTC_PLL_SET:
		tprints(", ");
		decode_rtc_pll_info(tcp, arg);
		break;
	case RTC_VL_READ:
		if (entering(tcp))
			return 0;
		tprints(", ");
		decode_rtc_vl(tcp, arg);
		break;
	case RTC_AIE_ON:
	case RTC_AIE_OFF:
	case RTC_UIE_ON:
	case RTC_UIE_OFF:
	case RTC_PIE_ON:
	case RTC_PIE_OFF:
	case RTC_WIE_ON:
	case RTC_WIE_OFF:
	case RTC_VL_CLR:
		/* no args */
		break;
	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
