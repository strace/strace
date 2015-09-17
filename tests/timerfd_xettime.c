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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/syscall.h>

#if defined __NR_timerfd_create \
 && defined __NR_timerfd_gettime \
 && defined __NR_timerfd_settime \
 && defined O_CLOEXEC

int
main(void)
{
	(void) close(0);
	if (syscall(__NR_timerfd_create, CLOCK_MONOTONIC, O_CLOEXEC | O_NONBLOCK))
		return 77;
	puts("timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK) = 0");

	struct {
		struct itimerspec its;
		uint32_t pad[4];
	} old = {
		.pad = { 0xdeadbeef, 0xbadc0ded, 0xdeadbeef, 0xbadc0ded }
	}, new = {
		.its = {
			.it_interval = { 0xdeface1, 0xdeface2 },
			.it_value = { 0xdeface3, 0xdeface4 }
		},
		.pad = { 0xdeadbeef, 0xbadc0ded, 0xdeadbeef, 0xbadc0ded }
	};

	if (syscall(__NR_timerfd_settime, 0, 0, &new.its, &old.its))
		return 77;
	printf("timerfd_settime(0, 0"
	       ", {it_interval={%jd, %jd}, it_value={%jd, %jd}}"
	       ", {it_interval={%jd, %jd}, it_value={%jd, %jd}}"
	       ") = 0\n",
	       (intmax_t) new.its.it_interval.tv_sec,
	       (intmax_t) new.its.it_interval.tv_nsec,
	       (intmax_t) new.its.it_value.tv_sec,
	       (intmax_t) new.its.it_value.tv_nsec,
	       (intmax_t) old.its.it_interval.tv_sec,
	       (intmax_t) old.its.it_interval.tv_nsec,
	       (intmax_t) old.its.it_value.tv_sec,
	       (intmax_t) old.its.it_value.tv_nsec);

	if (syscall(__NR_timerfd_gettime, 0, &old.its))
		return 77;
	printf("timerfd_gettime(0"
	       ", {it_interval={%jd, %jd}, it_value={%jd, %jd}}"
	       ") = 0\n",
	       (intmax_t) old.its.it_interval.tv_sec,
	       (intmax_t) old.its.it_interval.tv_nsec,
	       (intmax_t) old.its.it_value.tv_sec,
	       (intmax_t) old.its.it_value.tv_nsec);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
