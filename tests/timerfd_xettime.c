/*
 * Check decoding of timerfd_create, timerfd_gettime, and timerfd_settime
 * syscalls.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_timerfd_gettime \
 && defined __NR_timerfd_settime

# include <stdio.h>
# include <stdint.h>
# include <time.h>
# include <unistd.h>
# include "kernel_fcntl.h"

int
main(void)
{
	(void) close(0);
	if (syscall(__NR_timerfd_create, CLOCK_MONOTONIC, O_CLOEXEC | O_NONBLOCK))
		perror_msg_and_skip("timerfd_create");
	puts("timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK) = 0");

	TAIL_ALLOC_OBJECT_CONST_PTR(struct itimerspec, its_new);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct itimerspec, its_old);

	its_new->it_interval.tv_sec = 0xdeadbeefU;
	its_new->it_interval.tv_nsec = 0xfacefeedU;
	its_new->it_value.tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	its_new->it_value.tv_nsec = (long) 0xbadc0dedfacefeedLL;

	long rc = syscall(__NR_timerfd_settime, 0, 0, its_new, its_old);
	printf("timerfd_settime(0, 0"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}, %p) = %s\n",
	       (long long) its_new->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_new->it_interval.tv_nsec),
	       (long long) its_new->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_new->it_value.tv_nsec),
	       its_old, sprintrc(rc));

	its_new->it_interval.tv_sec = 0xdeface1;
	its_new->it_interval.tv_nsec = 0xdeface2;
	its_new->it_value.tv_sec = 0xdeface3;
	its_new->it_value.tv_nsec = 0xdeface4;
	its_old->it_interval.tv_sec = 0xdeface5;
	its_old->it_interval.tv_nsec = 0xdeface6;
	its_old->it_value.tv_sec = 0xdeface7;
	its_old->it_value.tv_nsec = 0xdeface8;

	if (syscall(__NR_timerfd_settime, 0, 0, its_new, its_old))
		perror_msg_and_skip("timerfd_settime");
	printf("timerfd_settime(0, 0"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}"
	       ") = 0\n",
	       (long long) its_new->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_new->it_interval.tv_nsec),
	       (long long) its_new->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_new->it_value.tv_nsec),
	       (long long) its_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_interval.tv_nsec),
	       (long long) its_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_value.tv_nsec));

	if (syscall(__NR_timerfd_gettime, 0, its_old))
		perror_msg_and_skip("timerfd_gettime");
	printf("timerfd_gettime(0"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}) = 0\n",
	       (long long) its_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_interval.tv_nsec),
	       (long long) its_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_value.tv_nsec));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_timerfd_gettime && __NR_timerfd_settime")

#endif
