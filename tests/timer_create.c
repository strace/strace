/*
 * Check decoding of timer_create syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include "sigevent.h"

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
		.sigev_value.sival_ptr =
			(void *) (unsigned long) 0xdeadbeefbadc0dedULL
	};

	syscall(__NR_timer_create, CLOCK_REALTIME, &sev, NULL);
	printf("timer_create(CLOCK_REALTIME, {sigev_value={sival_int=%d, "
	       "sival_ptr=%p}, sigev_signo=%u, "
	       "sigev_notify=%#x /* SIGEV_??? */}, NULL) = -1 %s (%m)\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_signo, sev.sigev_notify,
	       errno2name());

	sev.sigev_notify = SIGEV_NONE;
	if (syscall(__NR_timer_create, CLOCK_REALTIME, &sev, &tid[0]))
		perror_msg_and_skip("timer_create CLOCK_REALTIME");
	printf("timer_create(CLOCK_REALTIME, {sigev_value={sival_int=%d, "
	       "sival_ptr=%p}, sigev_signo=%u, sigev_notify=SIGEV_NONE}, "
	       "[%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_signo, tid[0]);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGALRM;
	if (syscall(__NR_timer_create, CLOCK_MONOTONIC, &sev, &tid[1]))
		perror_msg_and_skip("timer_create CLOCK_MONOTONIC");
	printf("timer_create(CLOCK_MONOTONIC, {sigev_value={sival_int=%d, "
	       "sival_ptr=%p}, sigev_signo=SIGALRM, "
	       "sigev_notify=SIGEV_SIGNAL}, [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr, tid[1]);

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_un.sigev_thread.function =
		(void *) (unsigned long) 0xdeadbeefbadc0dedULL;
	sev.sigev_un.sigev_thread.attribute =
		(void *) (unsigned long) 0xcafef00dfacefeedULL;
	if (syscall(__NR_timer_create, CLOCK_REALTIME, &sev, &tid[2]))
		perror_msg_and_skip("timer_create CLOCK_REALTIME");
	printf("timer_create(CLOCK_REALTIME, {sigev_value={sival_int=%d, "
	       "sival_ptr=%p}, sigev_signo=SIGALRM, sigev_notify=SIGEV_THREAD"
	       ", sigev_notify_function=%p, sigev_notify_attributes=%p}"
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
	       "sival_ptr=%p}, sigev_signo=SIGALRM, "
	       "sigev_notify=SIGEV_THREAD_ID, sigev_notify_thread_id=%d}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_un.tid,
	       tid[3]);

	puts("+++ exited with 0 +++");
	return 0;
}
