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
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <asm/unistd.h>

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
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = -1 EINVAL (%m)\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime);

	tv->tv_sec = 0xdeadbeefU;
	tv->tv_usec = 0xfacefeedU;
	assert(syscall(__NR_settimeofday, tv, tz) == -1);
	printf("settimeofday({tv_sec=%lld, tv_usec=%llu}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = -1 EINVAL (%m)\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime);

	tv->tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	tv->tv_usec = (long) 0xbadc0dedfacefeedLL;
	assert(syscall(__NR_settimeofday, tv, tz) == -1);
	printf("settimeofday({tv_sec=%lld, tv_usec=%llu}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = -1 EINVAL (%m)\n",
	       (long long) tv->tv_sec,
	       zero_extend_signed_to_ull(tv->tv_usec),
	       tz->tz_minuteswest, tz->tz_dsttime);

	puts("+++ exited with 0 +++");
	return 0;
}
