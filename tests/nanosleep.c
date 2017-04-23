/*
 * Check decoding of nanosleep syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
