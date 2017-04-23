/*
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
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>
#include <asm/unistd.h>

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
	syscall(__NR_clock_settime, CLOCK_THREAD_CPUTIME_ID, &t.ts);
	printf("clock_settime(CLOCK_THREAD_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = -1 EINVAL (%m)\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec));

	t.ts.tv_sec = 0xdeadbeefU;
	t.ts.tv_nsec = 0xfacefeedU;
	syscall(__NR_clock_settime, CLOCK_THREAD_CPUTIME_ID, &t.ts);
	printf("clock_settime(CLOCK_THREAD_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = -1 EINVAL (%m)\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec));

	t.ts.tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	t.ts.tv_nsec = (long) 0xbadc0dedfacefeedLL;
	syscall(__NR_clock_settime, CLOCK_THREAD_CPUTIME_ID, &t.ts);
	printf("clock_settime(CLOCK_THREAD_CPUTIME_ID"
	       ", {tv_sec=%lld, tv_nsec=%llu}) = -1 EINVAL (%m)\n",
	       (long long) t.ts.tv_sec,
	       zero_extend_signed_to_ull(t.ts.tv_nsec));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_clock_getres && __NR_clock_gettime && __NR_clock_settime")

#endif
