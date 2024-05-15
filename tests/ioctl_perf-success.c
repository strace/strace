/*
 * Check decoding of successful PERF_EVENT_IOC_{ID,QUERY_BPF} ioctls.
 *
 * Copyright (c) 2018-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>

int
main(int argc, char **argv)
{
	static const uint64_t magic64 = 0xfacefeeddeadc0deULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, u64_ptr);
	uint64_t *const u64_efault = u64_ptr + 1;
	TAIL_ALLOC_OBJECT_CONST_ARR(uint32_t, u32_arr, 4);
	uint32_t *const u32_efault = u32_arr + 4;

	unsigned long num_skip;
	long inject_retval;
	bool locked = false;

	*u64_ptr = magic64;

	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);
	inject_retval = strtol(argv[2], NULL, 0);

	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (unsigned long i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, PERF_EVENT_IOC_ID, NULL);

		printf("ioctl(-1, PERF_EVENT_IOC_ID, NULL) = %s%s\n",
		       sprintrc(ret),
		       ret == inject_retval ? " (INJECTED)" : "");

		if (ret != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", PERF_EVENT_IOC_ID, NULL) returning %lu",
				   inject_retval);

	/* PERF_EVENT_IOC_ID */
	assert(ioctl(-1, PERF_EVENT_IOC_ID, NULL) == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, NULL) = %ld (INJECTED)\n",
	       inject_retval);

	assert(ioctl(-1, PERF_EVENT_IOC_ID, u64_efault) == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, %p) = %ld (INJECTED)\n",
	       u64_efault, inject_retval);

	assert(ioctl(-1, PERF_EVENT_IOC_ID, u64_ptr) == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, [%" PRIu64 "]) = %ld (INJECTED)\n",
	       magic64, inject_retval);

	/* PERF_EVENT_IOC_QUERY_BPF */
	assert(ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, NULL) == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, NULL) = %ld (INJECTED)\n",
	       inject_retval);

	assert(ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_efault)
	       == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, %p) = %ld (INJECTED)\n",
	       u32_efault, inject_retval);

	u32_arr[3] = 0xdeadbeef;
	assert(ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_arr + 3)
	       == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, {ids_len=3735928559, ...}) "
	       "= %ld (INJECTED)\n",
	       inject_retval);

	u32_arr[2] = 0xdecaffed;
	assert(ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_arr + 2)
	       == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, {ids_len=3737845741"
	       ", prog_cnt=3735928559, ids=%p})"
	       " = %ld (INJECTED)\n",
	       u32_efault, inject_retval);

	u32_arr[0] = 0xbadc0ded;
	u32_arr[1] = 5;
	assert(ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_arr) == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, {ids_len=3134983661"
	       ", prog_cnt=5, ids=[3737845741, 3735928559, ... /* %p */]})"
	       " = %ld (INJECTED)\n",
	       u32_efault, inject_retval);

	u32_arr[1] = 2;
	assert(ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_arr) == inject_retval);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, {ids_len=3134983661"
	       ", prog_cnt=2, ids=[3737845741, 3735928559]})"
	       " = %ld (INJECTED)\n",
	       inject_retval);

	puts("+++ exited with 0 +++");
	return 0;
}
