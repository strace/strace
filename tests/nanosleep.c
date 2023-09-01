/*
 * Check decoding of nanosleep syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_nanosleep

# include <assert.h>
# include <stdio.h>
# include <stdint.h>
# include <signal.h>
# include <sys/time.h>
# include <unistd.h>

# include "kernel_old_timespec.h"

static const char *errstr;

static long
k_nanosleep(const void *const req, void *const rem)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = (uintptr_t) req;
	const kernel_ulong_t arg2 = (uintptr_t) rem;
	const long rc = syscall(__NR_nanosleep, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static void
handler(int signo)
{
}

int
main(void)
{
	struct {
		kernel_old_timespec_t ts;
		uint32_t pad[2];
	} req = {
		.ts.tv_nsec = 0xc0de1,
		.pad = { 0xdeadbeef, 0xbadc0ded }
	}, rem = {
		.ts = { .tv_sec = 0xc0de2, .tv_nsec = 0xc0de3 },
		.pad = { 0xdeadbeef, 0xbadc0ded }
	};
	const sigset_t set = {};
	const struct sigaction act = { .sa_handler = handler };
	const struct itimerval itv = { .it_value.tv_usec = 111111 };

	if (k_nanosleep(&req.ts, NULL))
		perror_msg_and_fail("nanosleep");
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, NULL) = 0\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec));

	assert(k_nanosleep(NULL, &rem.ts) == -1);
	printf("nanosleep(NULL, %p) = %s\n", &rem.ts, errstr);

	if (k_nanosleep(&req.ts, &rem.ts))
		perror_msg_and_fail("nanosleep");
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = 0\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts);

	req.ts.tv_nsec = 1000000000;
	assert(k_nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts,
	       errstr);

	req.ts.tv_sec = 0xdeadbeefU;
	req.ts.tv_nsec = 0xfacefeedU;
	assert(k_nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts,
	       errstr);

	req.ts.tv_sec = (typeof(req.ts.tv_sec)) 0xcafef00ddeadbeefLL;
	req.ts.tv_nsec = (long) 0xbadc0dedfacefeedLL;
	assert(k_nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts,
	       errstr);

	req.ts.tv_sec = -1;
	req.ts.tv_nsec = -1;
	assert(k_nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = %s\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts,
	       errstr);

	assert(sigaction(SIGALRM, &act, NULL) == 0);
	assert(sigprocmask(SIG_SETMASK, &set, NULL) == 0);

	if (setitimer(ITIMER_REAL, &itv, NULL))
		perror_msg_and_skip("setitimer");

	req.ts.tv_sec = 0;
	req.ts.tv_nsec = 999999999;
	assert(k_nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}"
	       ", {tv_sec=%lld, tv_nsec=%llu})"
	       " = ? ERESTART_RESTARTBLOCK (Interrupted by signal)\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec),
	       (long long) rem.ts.tv_sec,
	       zero_extend_signed_to_ull(rem.ts.tv_nsec));
	puts("--- SIGALRM {si_signo=SIGALRM, si_code=SI_KERNEL} ---");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_nanosleep")

#endif
