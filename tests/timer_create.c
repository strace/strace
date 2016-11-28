/*
 * This file is part of timer_create strace test.
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

#ifdef __NR_timer_create

# include <stdio.h>
# include <signal.h>
# include <time.h>
# include <unistd.h>
# include "sigevent.h"

int
main(void)
{
	syscall(__NR_timer_create, CLOCK_REALTIME, NULL, NULL);
	printf("timer_create(CLOCK_REALTIME, NULL, NULL) = -1 %s (%m)\n",
	       errno2name());

	int tid[4] = {};
	struct_sigevent sev = {
		.sigev_notify = 0xdefaced,
		.sigev_signo = 0xfacefeed,
		.sigev_value.sival_ptr = (unsigned long) 0xdeadbeefbadc0dedULL
	};

	syscall(__NR_timer_create, CLOCK_REALTIME, &sev, NULL);
	printf("timer_create(CLOCK_REALTIME, {sigev_value={sival_int=%d, "
	       "sival_ptr=%#lx}, sigev_signo=%u, "
	       "sigev_notify=%#x /* SIGEV_??? */}, NULL) = -1 %s (%m)\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_signo, sev.sigev_notify,
	       errno2name());

	sev.sigev_notify = SIGEV_NONE;
	if (syscall(__NR_timer_create, CLOCK_REALTIME, &sev, &tid[0]))
		perror_msg_and_skip("timer_create CLOCK_REALTIME");
	printf("timer_create(CLOCK_REALTIME, {sigev_value={sival_int=%d, "
	       "sival_ptr=%#lx}, sigev_signo=%u, sigev_notify=SIGEV_NONE}, "
	       "[%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_signo, tid[0]);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGALRM;
	if (syscall(__NR_timer_create, CLOCK_MONOTONIC, &sev, &tid[1]))
		perror_msg_and_skip("timer_create CLOCK_MONOTONIC");
	printf("timer_create(CLOCK_MONOTONIC, {sigev_value={sival_int=%d, "
	       "sival_ptr=%#lx}, sigev_signo=SIGALRM, "
	       "sigev_notify=SIGEV_SIGNAL}, [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr, tid[1]);

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_un.sigev_thread.function =
		(unsigned long) 0xdeadbeefbadc0dedULL;
	sev.sigev_un.sigev_thread.attribute =
		(unsigned long) 0xcafef00dfacefeedULL;
	if (syscall(__NR_timer_create, CLOCK_REALTIME, &sev, &tid[2]))
		perror_msg_and_skip("timer_create CLOCK_REALTIME");
	printf("timer_create(CLOCK_REALTIME, {sigev_value={sival_int=%d, "
	       "sival_ptr=%#lx}, sigev_signo=SIGALRM, sigev_notify=SIGEV_THREAD"
	       ", sigev_notify_function=%#lx, sigev_notify_attributes=%#lx}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_un.sigev_thread.function,
	       sev.sigev_un.sigev_thread.attribute,
	       tid[2]);

#ifndef SIGEV_THREAD_ID
# define SIGEV_THREAD_ID 4
#endif
	sev.sigev_notify = SIGEV_THREAD_ID;
	sev.sigev_un.tid = getpid();
	if (syscall(__NR_timer_create, CLOCK_MONOTONIC, &sev, &tid[3]))
		perror_msg_and_skip("timer_create CLOCK_MONOTONIC");
	printf("timer_create(CLOCK_MONOTONIC, {sigev_value={sival_int=%d, "
	       "sival_ptr=%#lx}, sigev_signo=SIGALRM, "
	       "sigev_notify=SIGEV_THREAD_ID, sigev_notify_thread_id=%d}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_un.tid,
	       tid[3]);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_timer_create")

#endif
