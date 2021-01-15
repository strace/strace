/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004-2016 Dmitry V. Levin <ldv@strace.io>
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

#include "print_fields.h"

#include "xlat/rtc_vl_flags.h"

#define XLAT_MACROS_ONLY
# include "xlat/rtc_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static void
print_rtc_time(struct tcb *tcp, const struct rtc_time *rt)
{
	PRINT_FIELD_D("{", *rt, tm_sec);
	PRINT_FIELD_D(", ", *rt, tm_min);
	PRINT_FIELD_D(", ", *rt, tm_hour);
	PRINT_FIELD_D(", ", *rt, tm_mday);
	PRINT_FIELD_D(", ", *rt, tm_mon);
	PRINT_FIELD_D(", ", *rt, tm_year);
	if (abbrev(tcp)) {
		tprints(", ...");
	} else {
		PRINT_FIELD_D(", ", *rt, tm_wday);
		PRINT_FIELD_D(", ", *rt, tm_yday);
		PRINT_FIELD_D(", ", *rt, tm_isdst);
	}
	tprints("}");
}

#define PRINT_FIELD_RTC_TIME(prefix_, where_, field_, tcp_)	\
	do {							\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);	\
		print_rtc_time((tcp_), &(where_).field_);	\
	} while (0)

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

	if (umove_or_printaddr(tcp, addr, &wk))
		return;

	PRINT_FIELD_U("{", wk, enabled);
	PRINT_FIELD_U(", ", wk, pending);
	PRINT_FIELD_RTC_TIME(", ", wk, time, tcp);
	tprints("}");
}

static void
decode_rtc_pll_info(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_rtc_pll_info pll;

	if (umove_or_printaddr(tcp, addr, &pll))
		return;

	PRINT_FIELD_D("{", pll, pll_ctrl);
	PRINT_FIELD_D(", ", pll, pll_value);
	PRINT_FIELD_D(", ", pll, pll_max);
	PRINT_FIELD_D(", ", pll, pll_min);
	PRINT_FIELD_D(", ", pll, pll_posmult);
	PRINT_FIELD_D(", ", pll, pll_negmult);
	PRINT_FIELD_D(", ", pll, pll_clock);
	tprints("}");
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
