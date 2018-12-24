/*
 * Check decoding of nanosleep syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>

static void
handler(int signo)
{
}

int
main(void)
{
	struct {
		struct timespec ts;
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

	if (nanosleep(&req.ts, NULL))
		perror_msg_and_fail("nanosleep");
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, NULL) = 0\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec));

	assert(nanosleep(NULL, &rem.ts) == -1);
	printf("nanosleep(NULL, %p) = -1 EFAULT (%m)\n", &rem.ts);

	if (nanosleep(&req.ts, &rem.ts))
		perror_msg_and_fail("nanosleep");
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = 0\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts);

	req.ts.tv_nsec = 1000000000;
	assert(nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = -1 EINVAL (%m)\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts);

	req.ts.tv_sec = 0xdeadbeefU;
	req.ts.tv_nsec = 0xfacefeedU;
	assert(nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = -1 EINVAL (%m)\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts);

	req.ts.tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	req.ts.tv_nsec = (long) 0xbadc0dedfacefeedLL;
	assert(nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = -1 EINVAL (%m)\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts);

	req.ts.tv_sec = -1;
	req.ts.tv_nsec = -1;
	assert(nanosleep(&req.ts, &rem.ts) == -1);
	printf("nanosleep({tv_sec=%lld, tv_nsec=%llu}, %p) = -1 EINVAL (%m)\n",
	       (long long) req.ts.tv_sec,
	       zero_extend_signed_to_ull(req.ts.tv_nsec), &rem.ts);

	assert(sigaction(SIGALRM, &act, NULL) == 0);
	assert(sigprocmask(SIG_SETMASK, &set, NULL) == 0);

	if (setitimer(ITIMER_REAL, &itv, NULL))
		perror_msg_and_skip("setitimer");

	req.ts.tv_sec = 0;
	req.ts.tv_nsec = 999999999;
	assert(nanosleep(&req.ts, &rem.ts) == -1);
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
