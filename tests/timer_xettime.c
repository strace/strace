/*
 * This file is part of timer_xettime strace test.
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
#include <asm/unistd.h>

#if defined __NR_timer_create \
 && defined __NR_timer_gettime \
 && defined __NR_timer_settime

# include <stdio.h>
# include <stdint.h>
# include <signal.h>
# include <time.h>
# include <unistd.h>

int
main(void)
{
	syscall(__NR_timer_settime, 0xdefaced, TIMER_ABSTIME, NULL, NULL);
	printf("timer_settime(%d, TIMER_ABSTIME, NULL, NULL)"
	       " = -1 EINVAL (%m)\n", 0xdefaced);

	long rc;
	int tid;
	struct sigevent sev = { .sigev_notify = SIGEV_NONE };

	if (syscall(__NR_timer_create, CLOCK_MONOTONIC, &sev, &tid))
		perror_msg_and_skip("timer_create");
	printf("timer_create(CLOCK_MONOTONIC, {sigev_signo=0"
	       ", sigev_notify=SIGEV_NONE}, [%d]) = 0\n", tid);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct itimerspec, its_new);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct itimerspec, its_old);

	its_new->it_interval.tv_sec = 0xdeadbeefU;
	its_new->it_interval.tv_nsec = 0xfacefeedU;
	its_new->it_value.tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	its_new->it_value.tv_nsec = (long) 0xbadc0dedfacefeedLL;

	rc = syscall(__NR_timer_settime, tid, 0, its_new, its_old);
	printf("timer_settime(%d, 0"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}, %p) = %s\n",
	       tid, (long long) its_new->it_interval.tv_sec,
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

	if (syscall(__NR_timer_settime, tid, 0, its_new, its_old))
		perror_msg_and_skip("timer_settime");
	printf("timer_settime(%d, 0"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}"
	       ") = 0\n",
	       tid,
	       (long long) its_new->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_new->it_interval.tv_nsec),
	       (long long) its_new->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_new->it_value.tv_nsec),
	       (long long) its_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_interval.tv_nsec),
	       (long long) its_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_value.tv_nsec));

	if (syscall(__NR_timer_gettime, tid, its_old))
		perror_msg_and_skip("timer_gettime");
	printf("timer_gettime(%d"
	       ", {it_interval={tv_sec=%lld, tv_nsec=%llu}"
	       ", it_value={tv_sec=%lld, tv_nsec=%llu}}) = 0\n",
	       tid,
	       (long long) its_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_interval.tv_nsec),
	       (long long) its_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(its_old->it_value.tv_nsec));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_timer_create && __NR_timer_gettime && __NR_timer_settime")

#endif
