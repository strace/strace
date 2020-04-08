/*
 * This file is part of ioctl_rtc strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/rtc.h>
#include "xlat.h"

static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;

static void
print_rtc_time(const struct rtc_time *rt)
{
	printf("{tm_sec=%d, tm_min=%d, tm_hour=%d"
	       ", tm_mday=%d, tm_mon=%d, tm_year=%d",
	       rt->tm_sec, rt->tm_min, rt->tm_hour,
	       rt->tm_mday, rt->tm_mon, rt->tm_year);
#if VERBOSE
	printf(", tm_wday=%d, tm_yday=%d, tm_isdst=%d}",
	       rt->tm_wday, rt->tm_yday, rt->tm_isdst);
#else
	printf(", ...}");
#endif
}

static struct xlat_data rtc_argless[] = {
	XLAT(RTC_AIE_OFF),
	XLAT(RTC_PIE_ON),
	XLAT(RTC_PIE_OFF),
	XLAT(RTC_UIE_ON),
	XLAT(RTC_WIE_ON),
	XLAT(RTC_WIE_OFF),
#ifdef RTC_VL_CLR
	XLAT(RTC_VL_CLR),
#endif
};

int
main(void)
{
	const unsigned int size = get_page_size();

	void *const page = tail_alloc(size);
	fill_memory(page, size);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct rtc_time, rt);
	fill_memory(rt, sizeof(*rt));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct rtc_wkalrm, wk);
	fill_memory(wk, sizeof(*wk));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct rtc_pll_info, pll);
	fill_memory(pll, sizeof(*pll));

	/* RTC_ALM_READ */
	ioctl(-1, RTC_ALM_READ, 0);
	printf("ioctl(-1, RTC_ALM_READ, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_ALM_READ, page);
	printf("ioctl(-1, RTC_ALM_READ, %p) = -1 EBADF (%m)\n", page);

	/* RTC_RD_TIME */
	ioctl(-1, RTC_RD_TIME, 0);
	printf("ioctl(-1, RTC_RD_TIME, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_RD_TIME, page);
	printf("ioctl(-1, RTC_RD_TIME, %p) = -1 EBADF (%m)\n", page);

	/* RTC_ALM_SET */
	ioctl(-1, RTC_ALM_SET, 0);
	printf("ioctl(-1, RTC_ALM_SET, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_ALM_SET, rt);
	printf("ioctl(-1, RTC_ALM_SET, ");
	print_rtc_time(rt);
	errno = EBADF;
	printf(") = -1 EBADF (%m)\n");

	/* RTC_SET_TIME */
	ioctl(-1, RTC_SET_TIME, 0);
	printf("ioctl(-1, RTC_SET_TIME, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_SET_TIME, rt);
	printf("ioctl(-1, RTC_SET_TIME, ");
	print_rtc_time(rt);
	errno = EBADF;
	printf(") = -1 EBADF (%m)\n");

	/* RTC_IRQP_SET */
	ioctl(-1, RTC_IRQP_SET, lmagic);
	printf("ioctl(-1, RTC_IRQP_SET, %lu) = -1 EBADF (%m)\n", lmagic);

	/* RTC_EPOCH_SET */
	ioctl(-1, RTC_EPOCH_SET, lmagic);
	printf("ioctl(-1, RTC_EPOCH_SET, %lu) = -1 EBADF (%m)\n", lmagic);

	/* RTC_IRQP_READ */
	ioctl(-1, RTC_IRQP_READ, 0);
	printf("ioctl(-1, RTC_IRQP_READ, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_IRQP_READ, page);
	printf("ioctl(-1, RTC_IRQP_READ, %p) = -1 EBADF (%m)\n", page);

	/* RTC_EPOCH_READ */
	ioctl(-1, RTC_EPOCH_READ, 0);
	printf("ioctl(-1, RTC_EPOCH_READ, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_EPOCH_READ, page);
	printf("ioctl(-1, RTC_EPOCH_READ, %p) = -1 EBADF (%m)\n", page);

	/* RTC_WKALM_RD */
	ioctl(-1, RTC_WKALM_RD, 0);
	printf("ioctl(-1, RTC_WKALM_RD, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_WKALM_RD, page);
	printf("ioctl(-1, RTC_WKALM_RD, %p) = -1 EBADF (%m)\n", page);

	/* RTC_WKALM_SET */
	ioctl(-1, RTC_WKALM_SET, 0);
	printf("ioctl(-1, RTC_WKALM_SET, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_WKALM_SET, wk);
	printf("ioctl(-1, RTC_WKALM_SET, {enabled=%u, pending=%u, time=",
	       (unsigned) wk->enabled, (unsigned) wk->pending);
	print_rtc_time(&wk->time);
	errno = EBADF;
	printf("}) = -1 EBADF (%m)\n");

	/* RTC_PLL_GET */
	ioctl(-1, RTC_PLL_GET, 0);
	printf("ioctl(-1, RTC_PLL_GET, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_PLL_GET, page);
	printf("ioctl(-1, RTC_PLL_GET, %p) = -1 EBADF (%m)\n", page);

	/* RTC_PLL_SET */
	ioctl(-1, RTC_PLL_SET, 0);
	printf("ioctl(-1, RTC_PLL_SET, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_PLL_SET, pll);
	printf("ioctl(-1, RTC_PLL_SET, {pll_ctrl=%d, pll_value=%d"
	       ", pll_max=%d, pll_min=%d, pll_posmult=%d, pll_negmult=%d"
	       ", pll_clock=%ld}) = -1 EBADF (%m)\n",
	       pll->pll_ctrl, pll->pll_value, pll->pll_max, pll->pll_min,
	       pll->pll_posmult, pll->pll_negmult, pll->pll_clock);

#ifdef RTC_VL_READ
	/* RTC_VL_READ */
	ioctl(-1, RTC_VL_READ, 0);
	printf("ioctl(-1, RTC_VL_READ, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, RTC_VL_READ, page);
	printf("ioctl(-1, RTC_VL_READ, %p) = -1 EBADF (%m)\n", page);
#endif

	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(rtc_argless); ++i) {
		ioctl(-1, (unsigned long) rtc_argless[i].val, lmagic);
		printf("ioctl(-1, %s) = -1 EBADF (%m)\n", rtc_argless[i].str);
	}

	ioctl(-1, RTC_UIE_OFF, lmagic);
	printf("ioctl(-1, %s) = -1 EBADF (%m)\n", "PHN_NOT_OH or RTC_UIE_OFF");

	ioctl(-1, RTC_AIE_ON, lmagic);
#ifdef HPPA
	printf("ioctl(-1, %s) = -1 EBADF (%m)\n", "PA_PERF_ON or RTC_AIE_ON");
#else
	printf("ioctl(-1, %s) = -1 EBADF (%m)\n", "RTC_AIE_ON");
#endif

	ioctl(-1, _IO(0x70, 0x40), lmagic);
	printf("ioctl(-1, %s, %#lx) = -1 EBADF (%m)\n", "NVRAM_INIT", lmagic);

	puts("+++ exited with 0 +++");
	return 0;
}
