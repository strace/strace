/*
 * Linux UAPI clock IDs and names for the printclockname tests.
 *
 * This separate compilation unit includes <linux/time.h> for the clock IDs
 * but no libc headers: their time structures and macros can conflict with
 * the UAPI definitions.  Test programs can include libc headers without
 * including <linux/time.h> themselves.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "clock_ids_for_tests.h"
#include <linux/time.h>

#define CLOCK_TEST_ID(id_) { id_, #id_ }

const struct clock_test_id *
clock_ids_for_tests(unsigned int *count)
{
	static const struct clock_test_id ids[] = {
		CLOCK_TEST_ID(CLOCK_REALTIME),
		CLOCK_TEST_ID(CLOCK_MONOTONIC),
		CLOCK_TEST_ID(CLOCK_PROCESS_CPUTIME_ID),
		CLOCK_TEST_ID(CLOCK_THREAD_CPUTIME_ID),
		CLOCK_TEST_ID(CLOCK_MONOTONIC_RAW),
		CLOCK_TEST_ID(CLOCK_REALTIME_COARSE),
		CLOCK_TEST_ID(CLOCK_MONOTONIC_COARSE),
		CLOCK_TEST_ID(CLOCK_BOOTTIME),
		CLOCK_TEST_ID(CLOCK_REALTIME_ALARM),
		CLOCK_TEST_ID(CLOCK_BOOTTIME_ALARM),
		CLOCK_TEST_ID(CLOCK_SGI_CYCLE),
		CLOCK_TEST_ID(CLOCK_TAI),
		CLOCK_TEST_ID(CLOCK_AUX + 0),
		CLOCK_TEST_ID(CLOCK_AUX + 7),
	};

	*count = sizeof(ids) / sizeof(ids[0]);
	return ids;
}

unsigned int
clock_aux_last_for_tests(void)
{
	return CLOCK_AUX_LAST;
}
