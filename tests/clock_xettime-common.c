/*
 * This file is part of clock_xettime* strace tests.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include "kernel_timespec.h"

static const char *errstr;

static long
k_syscall(const unsigned int scno, const unsigned int id, void *const ts)
{
	static const kernel_ulong_t bad =
		(kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = id | fill;
	const kernel_ulong_t arg2 = f8ill_ptr_to_kulong(ts);
	const long rc = syscall(scno, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static long
k_getres(const unsigned int id, void *const ts)
{
	return k_syscall(SYSCALL_NR_getres, id, ts);
}

static long
k_gettime(const unsigned int id, void *const ts)
{
	return k_syscall(SYSCALL_NR_gettime, id, ts);
}

static long
k_settime(const unsigned int id, void *const ts)
{
	return k_syscall(SYSCALL_NR_settime, id, ts);
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(clock_timespec_t, ts);
	void *const efault = (void *) (1 + (char *) ts);

	k_getres(CLOCK_MONOTONIC, NULL);
	printf("%s(CLOCK_MONOTONIC, NULL) = %s\n", SYSCALL_NAME_getres, errstr);

	k_getres(CLOCK_REALTIME, efault);
	printf("%s(CLOCK_REALTIME, %p) = %s\n",
	       SYSCALL_NAME_getres, efault, errstr);

	k_gettime(CLOCK_MONOTONIC, NULL);
	printf("%s(CLOCK_MONOTONIC, NULL) = %s\n", SYSCALL_NAME_gettime, errstr);

	k_gettime(CLOCK_REALTIME, efault);
	printf("%s(CLOCK_REALTIME, %p) = %s\n",
	       SYSCALL_NAME_gettime, efault, errstr);

	k_settime(CLOCK_MONOTONIC, NULL);
	printf("%s(CLOCK_MONOTONIC, NULL) = %s\n", SYSCALL_NAME_settime, errstr);

	k_settime(CLOCK_REALTIME, efault);
	printf("%s(CLOCK_REALTIME, %p) = %s\n",
	       SYSCALL_NAME_settime, efault, errstr);

	if (k_getres(CLOCK_MONOTONIC, ts))
		perror_msg_and_skip("clock_getres CLOCK_MONOTONIC");
	printf("%s(CLOCK_MONOTONIC, {tv_sec=%lld, tv_nsec=%llu}) = 0\n",
	       SYSCALL_NAME_getres, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec));

	if (k_gettime(CLOCK_PROCESS_CPUTIME_ID, ts))
		perror_msg_and_skip("clock_gettime CLOCK_PROCESS_CPUTIME_ID");
	printf("%s(CLOCK_PROCESS_CPUTIME_ID, {tv_sec=%lld, tv_nsec=%llu})"
	       " = 0\n",
	       SYSCALL_NAME_gettime, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec));

	ts->tv_sec = 0xdeface1;
	ts->tv_nsec = 0xdeface2;
	k_settime(CLOCK_THREAD_CPUTIME_ID, ts);
	printf("%s(CLOCK_THREAD_CPUTIME_ID, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       SYSCALL_NAME_settime, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), errstr);

	ts->tv_sec = 0xdeadbeefU;
	ts->tv_nsec = 0xfacefeedU;
	k_settime(CLOCK_THREAD_CPUTIME_ID, ts);
	printf("%s(CLOCK_THREAD_CPUTIME_ID, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       SYSCALL_NAME_settime, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), errstr);

	ts->tv_sec = (typeof(ts->tv_sec)) 0xcafef00ddeadbeefLL;
	ts->tv_nsec = (typeof(ts->tv_nsec)) 0xbadc0dedfacefeedLL;
	k_settime(CLOCK_THREAD_CPUTIME_ID, ts);
	printf("%s(CLOCK_THREAD_CPUTIME_ID, {tv_sec=%lld, tv_nsec=%llu})"
	       " = %s\n",
	       SYSCALL_NAME_settime, (long long) ts->tv_sec,
	       zero_extend_signed_to_ull(ts->tv_nsec), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
