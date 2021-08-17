/*
 * Check decoding of prctl operations without arguments and return code parsing:
 * PR_GET_KEEPCAPS, PR_GET_SECCOMP, PR_GET_TIMERSLACK, PR_GET_TIMING,
 * PR_TASK_PERF_EVENTS_DISABLE, PR_TASK_PERF_EVENTS_ENABLE, and
 * PR_GET_TAGGED_ADDR_CTRL.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include <linux/prctl.h>

int
main(void)
{
	static const kernel_ulong_t bogus_op_bits =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	static const kernel_ulong_t bogus_arg =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL;
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} options[] = {
		{  7, "PR_GET_KEEPCAPS" },
		{ 13, "PR_GET_TIMING" },
		{ 21, "PR_GET_SECCOMP" },
		{ 30, "PR_GET_TIMERSLACK" },
		{ 31, "PR_TASK_PERF_EVENTS_DISABLE" },
		{ 32, "PR_TASK_PERF_EVENTS_ENABLE" },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, ptr);

	prctl_marker();

	for (unsigned int i = 0; i < ARRAY_SIZE(options); i++) {
		long rc = syscall(__NR_prctl, options[i].val | bogus_op_bits,
				  bogus_arg);
		printf("prctl(%s) = %s\n", options[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
