/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_gettimeofday

# include <assert.h>
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>
# include <sys/time.h>
# include "scno.h"

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct timeval, tv);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct timezone, tz);

	if (syscall(__NR_gettimeofday, tv, NULL))
		perror_msg_and_skip("gettimeofday");
	printf("gettimeofday({tv_sec=%lld, tv_usec=%llu}, NULL) = 0\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec));

	if (syscall(__NR_gettimeofday, tv, tz))
		perror_msg_and_skip("gettimeofday");
	printf("gettimeofday({tv_sec=%lld, tv_usec=%llu}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = 0\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime);

	tv->tv_sec = -1;
	tv->tv_usec = 1000000;
	assert(syscall(__NR_settimeofday, tv, tz) == -1);
	printf("settimeofday({tv_sec=%lld, tv_usec=%llu}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = %s\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime, sprintrc(-1));

	tv->tv_sec = 0xdeadbeefU;
	tv->tv_usec = 0xfacefeedU;
	assert(syscall(__NR_settimeofday, tv, tz) == -1);
	printf("settimeofday({tv_sec=%lld, tv_usec=%llu}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = %s\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime, sprintrc(-1));

	tv->tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	tv->tv_usec = (suseconds_t) 0xbadc0dedfacefeedLL;
	assert(syscall(__NR_settimeofday, tv, tz) == -1);
	printf("settimeofday({tv_sec=%lld, tv_usec=%llu}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = %s\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime, sprintrc(-1));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_gettimeofday")

#endif /* __NR_gettimeofday */
