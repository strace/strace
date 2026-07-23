/*
 * Check decoding of clock IDs in clock_getres.
 *
 * Copyright (c) 2015-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "clock_ids_for_tests.h"

#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static long
k_clock_getres(const unsigned int id, void *const ts)
{
	static const kernel_ulong_t bad =
		(kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = id | fill;
	const kernel_ulong_t arg2 = f8ill_ptr_to_kulong(ts);
	const long rc = syscall(__NR_clock_getres, arg1, arg2,
				bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	unsigned int const_clock_ids_count;
	const struct clock_test_id *const const_clock_ids =
		clock_ids_for_tests(&const_clock_ids_count);

	for (unsigned int i = 0; i < const_clock_ids_count; ++i) {
		k_clock_getres(const_clock_ids[i].val, NULL);
		printf("clock_getres(" XLAT_FMT ", NULL) = %s\n",
		       XLAT_SEL(const_clock_ids[i].val, const_clock_ids[i].str),
		       errstr);
	}

	static const struct strval32 dyn_clock_ids[] = {
		{ ARG_XLAT_UNKNOWN(-5, "FD_TO_CLOCKID(0)") },
		{ ARG_XLAT_UNKNOWN(-21, "FD_TO_CLOCKID(2)") },
		{ ARG_XLAT_UNKNOWN(-15,
				   "MAKE_PROCESS_CPUCLOCK(1, CPUCLOCK_VIRT)") },
		{ ARG_XLAT_UNKNOWN(-10,
				   "MAKE_THREAD_CPUCLOCK(1, CPUCLOCK_SCHED)") },
		{ ARG_XLAT_UNKNOWN(-9,
				   "MAKE_THREAD_CPUCLOCK(1, 0x3)") },
	};
	for (unsigned int i = 0; i < ARRAY_SIZE(dyn_clock_ids); ++i) {
		k_clock_getres(dyn_clock_ids[i].val, NULL);
		printf("clock_getres(%s, NULL) = %s\n",
		       dyn_clock_ids[i].str, errstr);
	}

	const unsigned int beyond_aux_clock = clock_aux_last_for_tests() + 1;
	k_clock_getres(beyond_aux_clock, NULL);
	printf("clock_getres(" XLAT_UNKNOWN_FMT("%#x", "CLOCK_???")
	       ", NULL) = %s\n", beyond_aux_clock, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
