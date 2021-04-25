/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
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
	tprint_struct_begin();
	PRINT_FIELD_D(*rt, tm_sec);
	tprint_struct_next();
	PRINT_FIELD_D(*rt, tm_min);
	tprint_struct_next();
	PRINT_FIELD_D(*rt, tm_hour);
	tprint_struct_next();
	PRINT_FIELD_D(*rt, tm_mday);
	tprint_struct_next();
	PRINT_FIELD_D(*rt, tm_mon);
	tprint_struct_next();
	PRINT_FIELD_D(*rt, tm_year);
	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		tprint_struct_next();
		PRINT_FIELD_D(*rt, tm_wday);
		tprint_struct_next();
		PRINT_FIELD_D(*rt, tm_yday);
		tprint_struct_next();
		PRINT_FIELD_D(*rt, tm_isdst);
	}
	tprint_struct_end();
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

	if (umove_or_printaddr(tcp, addr, &wk))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(wk, enabled);
	tprint_struct_next();
	PRINT_FIELD_U(wk, pending);
	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_PTR(wk, time, tcp, print_rtc_time);
	tprint_struct_end();
}

static void
decode_rtc_pll_info(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_rtc_pll_info pll;

	if (umove_or_printaddr(tcp, addr, &pll))
		return;

	tprint_struct_begin();
	PRINT_FIELD_D(pll, pll_ctrl);
	tprint_struct_next();
	PRINT_FIELD_D(pll, pll_value);
	tprint_struct_next();
	PRINT_FIELD_D(pll, pll_max);
	tprint_struct_next();
	PRINT_FIELD_D(pll, pll_min);
	tprint_struct_next();
	PRINT_FIELD_D(pll, pll_posmult);
	tprint_struct_next();
	PRINT_FIELD_D(pll, pll_negmult);
	tprint_struct_next();
	PRINT_FIELD_D(pll, pll_clock);
	tprint_struct_end();
}

static void
decode_rtc_vl(struct tcb *const tcp, const kernel_ulong_t addr)
{
	unsigned int val;

	if (umove_or_printaddr(tcp, addr, &val))
		return;

	tprint_indirect_begin();
	printflags(rtc_vl_flags, val, "RTC_VL_???");
	tprint_indirect_end();
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
		tprint_arg_next();
		decode_rtc_time(tcp, arg);
		break;
	case RTC_IRQP_SET:
	case RTC_EPOCH_SET:
		tprint_arg_next();
		PRINT_VAL_U(arg);
		break;
	case RTC_IRQP_READ:
	case RTC_EPOCH_READ:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_ulong(tcp, arg);
		break;
	case RTC_WKALM_RD:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case RTC_WKALM_SET:
		tprint_arg_next();
		decode_rtc_wkalrm(tcp, arg);
		break;
	case RTC_PLL_GET:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case RTC_PLL_SET:
		tprint_arg_next();
		decode_rtc_pll_info(tcp, arg);
		break;
	case RTC_VL_READ:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
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
