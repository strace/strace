/*
 * Check RTC_* ioctl decoding using success injection.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <sys/ioctl.h>

static const char *errstr;

#define INJECT_RETVAL 42

#ifndef RTC_VL_READ
# define RTC_VL_READ _IOR('p', 0x13, unsigned int)
#endif
#ifndef RTC_VL_CLR
# define RTC_VL_CLR _IO('p', 0x14)
#endif

static long
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	long rc = ioctl(-1, cmd, arg);

	errstr = sprintrc(rc);

	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %ld",
				   rc, (long) INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;

	return rc;
}

static inline long
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

int
main(int argc, char *argv[])
{
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, RTC_AIE_ON, 23);

		printf("ioctl(-1, %s) = %s%s\n",
		       XLAT_STR(RTC_AIE_ON), sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", RTC_AIE_ON, 23) returning %d",
				   INJECT_RETVAL);

	/* Unknown cmds */
	static const struct strval32 cmd_flags[] = {
		{ ARG_STR(_IOC_NONE) },
		{ ARG_STR(_IOC_READ) },
		{ ARG_STR(_IOC_WRITE) },
		{ ARG_STR(_IOC_READ|_IOC_WRITE) },
	};
	static const uint8_t unknown_cmds[] = { 0, 0x15, 0x16, 0x17, 0xff };

	for (size_t i = 0; i < ARRAY_SIZE(unknown_cmds); i++) {
		/* Trying to hit common argument sizes */
		for (size_t j = 0; j < 256; j += 4) {
			for (size_t k = 0; k < ARRAY_SIZE(cmd_flags); k++) {
				uint32_t ioc = _IOC(cmd_flags[k].val, 'p',
						    unknown_cmds[i], j);

				/*
				 * Conflicts with Phantom misc char dev driver
				 * that uses the same 'p' commant type.
				 */
				switch (ioc) {
				/* PHN_GET_REG */
				case _IOC(_IOC_READ|_IOC_WRITE, 0x70, 0,
					  sizeof(void *)):
				/* PKEY_VERIFYKEY2 */
#ifdef __s390__
				case _IOC(_IOC_READ|_IOC_WRITE, 0x70, 0x17,
					  0x18):
#endif
#ifdef __s390x__
				case _IOC(_IOC_READ|_IOC_WRITE, 0x70, 0x17,
					  0x20):
#endif
					continue;
				}

				do_ioctl(ioc, 0);
				printf("ioctl(-1, " NABBR("%#x") VERB(" /* ")
				       NRAW("_IOC(%s, 0x70, %#x, %#zx)")
				       VERB(" */") ", 0) = %s\n",
#if XLAT_RAW || XLAT_VERBOSE
				       ioc,
#endif
#if !XLAT_RAW
				       cmd_flags[k].str, unknown_cmds[i], j,
#endif
				       errstr);

				do_ioctl(ioc,
					 (unsigned long) 0xbadc0deddeadc0deULL);
				printf("ioctl(-1, " NABBR("%#x") VERB(" /* ")
				       NRAW("_IOC(%s, 0x70, %#x, %#zx)")
				       VERB(" */") ", %#lx) = %s\n",
#if XLAT_RAW || XLAT_VERBOSE
				       ioc,
#endif
#if !XLAT_RAW
				       cmd_flags[k].str, unknown_cmds[i], j,
#endif
				       (unsigned long) 0xbadc0deddeadc0deULL,
				       errstr);
			}
		}
	}


	/* Commands without argument */
	static const struct strval32 noarg_cmds[] = {
#ifdef HPPA
		{ RTC_AIE_ON, "PA_PERF_ON or RTC_AIE_ON" },
#else
		{ ARG_STR(RTC_AIE_ON) },
#endif
		{ ARG_STR(RTC_AIE_OFF) },
		{ ARG_STR(RTC_UIE_ON) },
		{ RTC_UIE_OFF, "PHN_NOT_OH or RTC_UIE_OFF" },
		{ ARG_STR(RTC_PIE_ON) },
		{ ARG_STR(RTC_PIE_OFF) },
		{ ARG_STR(RTC_WIE_ON) },
		{ ARG_STR(RTC_WIE_OFF) },
		{ ARG_STR(RTC_VL_CLR) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(noarg_cmds); i++) {
		do_ioctl(noarg_cmds[i].val, 42);
		printf("ioctl(-1, %s) = %s\n",
		       sprintxlat(noarg_cmds[i].str, noarg_cmds[i].val, NULL),
		       errstr);
	}


	/* RTC_ALM_SET, RTC_ALM_READ, RTC_RD_TIME, RTC_SET_TIME */
	static const struct strval32 rtct_cmds[] = {
		{ ARG_STR(RTC_ALM_SET) },
		{ ARG_STR(RTC_ALM_READ) },
		{ ARG_STR(RTC_RD_TIME) },
		{ ARG_STR(RTC_SET_TIME) },
	};

	struct rtc_time *rtct = tail_alloc(sizeof(*rtct));

	fill_memory32(rtct, sizeof(*rtct));

	for (size_t i = 0; i < ARRAY_SIZE(rtct_cmds); i++) {
		do_ioctl(rtct_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       sprintxlat(rtct_cmds[i].str, rtct_cmds[i].val, NULL),
		       errstr);

		do_ioctl_ptr(rtct_cmds[i].val, (char *) rtct + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       sprintxlat(rtct_cmds[i].str, rtct_cmds[i].val, NULL),
		       (char *) rtct + 1, errstr);

		do_ioctl_ptr(rtct_cmds[i].val, rtct);
		printf("ioctl(-1, %s, {tm_sec=-2136948512, tm_min=-2136948511"
		       ", tm_hour=-2136948510, tm_mday=-2136948509"
		       ", tm_mon=-2136948508, tm_year=-2136948507"
#if VERBOSE
		       ", tm_wday=-2136948506, tm_yday=-2136948505"
		       ", tm_isdst=-2136948504"
#else
		       ", ..."
#endif
		       "}) = %s\n",
		       sprintxlat(rtct_cmds[i].str, rtct_cmds[i].val, NULL),
		       errstr);
	}


	/* RTC_IRQP_READ, RTC_EPOCH_READ */
	static const struct strval32 lptr_cmds[] = {
		{ ARG_STR(RTC_IRQP_READ) },
		{ ARG_STR(RTC_EPOCH_READ) },
	};

	unsigned long *ulval = tail_alloc(sizeof(*ulval));

	*ulval = (unsigned long) 0xbadc0dedbeeffaceULL;

	for (size_t i = 0; i < ARRAY_SIZE(lptr_cmds); i++) {
		do_ioctl(lptr_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       sprintxlat(lptr_cmds[i].str, lptr_cmds[i].val, NULL),
		       errstr);

		do_ioctl_ptr(lptr_cmds[i].val, (char *) ulval + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       sprintxlat(lptr_cmds[i].str, lptr_cmds[i].val, NULL),
		       (char *) ulval + 1, errstr);

		do_ioctl_ptr(lptr_cmds[i].val, ulval);
		printf("ioctl(-1, %s, [%lu]) = %s\n",
		       sprintxlat(lptr_cmds[i].str, lptr_cmds[i].val, NULL),
		       (unsigned long) 0xbadc0dedbeeffaceULL, errstr);
	}


	/* RTC_IRQP_SET, RTC_EPOCH_SET */
	static const struct strval32 larg_cmds[] = {
		{ ARG_STR(RTC_IRQP_SET) },
		{ ARG_STR(RTC_EPOCH_SET) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(larg_cmds); i++) {
		do_ioctl(larg_cmds[i].val, 0);
		printf("ioctl(-1, %s, 0) = %s\n",
		       sprintxlat(larg_cmds[i].str, larg_cmds[i].val, NULL),
		       errstr);

		do_ioctl(larg_cmds[i].val,
		         (unsigned long) 0xbadc0dedbeeffaceULL);
		printf("ioctl(-1, %s, %lu) = %s\n",
		       sprintxlat(larg_cmds[i].str, larg_cmds[i].val, NULL),
		       (unsigned long) 0xbadc0dedbeeffaceULL, errstr);
	}


	/* RTC_WKALM_SET, RTC_WKALM_RD */
	static const struct strval32 rwka_cmds[] = {
		{ ARG_STR(RTC_WKALM_SET) },
		{ ARG_STR(RTC_WKALM_RD) },
	};

	struct rtc_wkalrm *rwka = tail_alloc(sizeof(*rwka));

	fill_memory(rwka, sizeof(*rwka));
	fill_memory32(&rwka->time, sizeof(rwka->time));

	for (size_t i = 0; i < ARRAY_SIZE(rwka_cmds); i++) {
		do_ioctl(rwka_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       sprintxlat(rwka_cmds[i].str, rwka_cmds[i].val, NULL),
		       errstr);

		do_ioctl_ptr(rwka_cmds[i].val, (char *) rwka + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       sprintxlat(rwka_cmds[i].str, rwka_cmds[i].val, NULL),
		       (char *) rwka + 1, errstr);

		do_ioctl_ptr(rwka_cmds[i].val, rwka);
		printf("ioctl(-1, %s, {enabled=128, pending=129"
		       ", time={tm_sec=-2136948512, tm_min=-2136948511"
		       ", tm_hour=-2136948510, tm_mday=-2136948509"
		       ", tm_mon=-2136948508, tm_year=-2136948507"
#if VERBOSE
		       ", tm_wday=-2136948506, tm_yday=-2136948505"
		       ", tm_isdst=-2136948504"
#else
		       ", ..."
#endif
		       "}}) = %s\n",
		       sprintxlat(rwka_cmds[i].str, rwka_cmds[i].val, NULL),
		       errstr);
	}


	/* RTC_PLL_GET, RTC_PLL_SET */
	static const struct strval32 rpll_cmds[] = {
		{ ARG_STR(RTC_PLL_GET) },
		{ ARG_STR(RTC_PLL_SET) },
	};

	struct rtc_pll_info *rpll = tail_alloc(sizeof(*rpll));

	fill_memory32(rpll, sizeof(*rpll));
	rpll->pll_clock = (unsigned long) 0xbadc0dedbeeffaceULL;

	for (size_t i = 0; i < ARRAY_SIZE(rpll_cmds); i++) {
		do_ioctl(rpll_cmds[i].val, 0);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       sprintxlat(rpll_cmds[i].str, rpll_cmds[i].val, NULL),
		       errstr);

		do_ioctl_ptr(rpll_cmds[i].val, (char *) rpll + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       sprintxlat(rpll_cmds[i].str, rpll_cmds[i].val, NULL),
		       (char *) rpll + 1, errstr);

		do_ioctl_ptr(rpll_cmds[i].val, rpll);
		printf("ioctl(-1, %s, {pll_ctrl=-2136948512"
		       ", pll_value=-2136948511, pll_max=-2136948510"
		       ", pll_min=-2136948509, pll_posmult=-2136948508"
		       ", pll_negmult=-2136948507, pll_clock=%ld}) = %s\n",
		       sprintxlat(rpll_cmds[i].str, rpll_cmds[i].val, NULL),
		       (unsigned long) 0xbadc0dedbeeffaceULL, errstr);
	}


	/* RTC_VL_READ */
	static const struct strval32 rvl_vals[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "RTC_VL_DATA_INVALID") },
		{ ARG_XLAT_KNOWN(0xdec0dedf, "RTC_VL_DATA_INVALID"
				 "|RTC_VL_BACKUP_LOW|RTC_VL_BACKUP_EMPTY"
				 "|RTC_VL_ACCURACY_LOW|0xdec0ded0") },
		{ ARG_XLAT_UNKNOWN(0xdec0ded0, "RTC_VL_???") },
	};

	unsigned int *uival = tail_alloc(sizeof(*uival));

	do_ioctl(RTC_VL_READ, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(RTC_VL_READ), errstr);

	do_ioctl_ptr(RTC_VL_READ, (char *) uival + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(RTC_VL_READ), (char *) uival + 1, errstr);

	for (size_t i = 0; i < ARRAY_SIZE(rvl_vals); i++) {
		*uival = rvl_vals[i].val;

		do_ioctl_ptr(RTC_VL_READ, uival);
		printf("ioctl(-1, %s, [%s]) = %s\n",
		       XLAT_STR(RTC_VL_READ), rvl_vals[i].str, errstr);
	}


	puts("+++ exited with 0 +++");
	return 0;
}
