/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/syscall.h>

int
main(void)
{
	struct {
		struct timeval tv;
		uint32_t pad0[2];
		struct timezone tz;
		uint32_t pad1[2];
	} t = {
		.pad0 = { 0xdeadbeef, 0xbadc0ded },
		.pad1 = { 0xdeadbeef, 0xbadc0ded }
	};

	if (syscall(__NR_gettimeofday, &t.tv, NULL))
		return 77;
	printf("gettimeofday({%jd, %jd}, NULL) = 0\n",
	       (intmax_t) t.tv.tv_sec, (intmax_t) t.tv.tv_usec);

	if (syscall(__NR_gettimeofday, &t.tv, &t.tz))
		return 77;
	printf("gettimeofday({%jd, %jd}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d}) = 0\n",
	       (intmax_t) t.tv.tv_sec, (intmax_t) t.tv.tv_usec,
	       t.tz.tz_minuteswest, t.tz.tz_dsttime);

	t.tv.tv_sec = -1;
	t.tv.tv_usec = 1000000000;
	if (!settimeofday(&t.tv, &t.tz))
		return 77;
	printf("settimeofday({%jd, %jd}"
	       ", {tz_minuteswest=%d, tz_dsttime=%d})"
	       " = -1 EINVAL (Invalid argument)\n",
	       (intmax_t) t.tv.tv_sec, (intmax_t) t.tv.tv_usec,
	       t.tz.tz_minuteswest, t.tz.tz_dsttime);

	puts("+++ exited with 0 +++");
	return 0;
}
