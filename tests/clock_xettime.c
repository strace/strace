/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include "scno.h"

#if defined __NR_clock_getres \
 && defined __NR_clock_gettime \
 && defined __NR_clock_settime

int
main(void)
{
	struct {
		struct timespec ts;
		uint32_t pad[2];
	} t = {
		.pad = { 0xdeadbeef, 0xbadc0ded }
	};
	long rc;

	if (syscall(__NR_clock_getres, CLOCK_REALTIME, &t.ts))
		perror_msg_and_skip("clock_getres CLOCK_REALTIME");
	printf("clock_getres(CLOCK_REALTIME, {tv_sec=%lld, tv_nsec=%llu})"
	       " = 0\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec));

	if (syscall(__NR_clock_gettime, CLOCK_PROCESS_CPUTIME_ID, &t.ts))
		perror_msg_and_skip("clock_gettime CLOCK_PROCESS_CPUTIME_ID");
	printf("clock_gettime(CLOCK_PROCESS_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = 0\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec));

	t.ts.tv_sec = 0xdeface1;
	t.ts.tv_nsec = 0xdeface2;
	rc = syscall(__NR_clock_settime, CLOCK_THREAD_CPUTIME_ID, &t.ts);
	printf("clock_settime(CLOCK_THREAD_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec), sprintrc(rc));

	t.ts.tv_sec = 0xdeadbeefU;
	t.ts.tv_nsec = 0xfacefeedU;
	rc = syscall(__NR_clock_settime, CLOCK_THREAD_CPUTIME_ID, &t.ts);
	printf("clock_settime(CLOCK_THREAD_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec), sprintrc(rc));

	t.ts.tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	t.ts.tv_nsec = (long) 0xbadc0dedfacefeedLL;
	rc = syscall(__NR_clock_settime, CLOCK_THREAD_CPUTIME_ID, &t.ts);
	printf("clock_settime(CLOCK_THREAD_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = %s\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec), sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_clock_getres && __NR_clock_gettime && __NR_clock_settime")

#endif
