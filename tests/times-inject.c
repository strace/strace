/*
 * Check clock tick formatting with deterministic times results.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/times.h>
#include <unistd.h>

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct tms, tbuf);
	/* Include rounding ties and the second boundary at 1024 Hz.  */
	static const uint64_t values[] = {
		0, 1, 3, 31, 32, 33, 96, 1023, 1024, 1056,
		UINT64_C(0x20000000000001), INT64_MAX,
	};

	for (size_t i = 0; i < ARRAY_SIZE(values); ++i) {
		const uint64_t val = values[i];

		if (sizeof(tbuf->tms_utime) < sizeof(val) && val > INT32_MAX)
			continue;

		tbuf->tms_utime = tbuf->tms_stime = val;
		tbuf->tms_cutime = tbuf->tms_cstime = val;
		long rc = syscall(__NR_times, tbuf);

		if (rc != 42)
			error_msg_and_fail("times: expected injected return value 42");

		char str[128];
#if XLAT_RAW
		snprintf(str, sizeof(str), "%" PRIu64, val);
#else
		clock_t_str(val, str, sizeof(str));
#endif
		printf("times({tms_utime=%s, tms_stime=%s, "
		       "tms_cutime=%s, tms_cstime=%s}) = 42 (INJECTED)\n",
		       str, str, str, str);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
