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
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

#ifdef __NR_timer_create

int
main(void)
{
	int tid[4] = {};
	struct sigevent sev = {
		.sigev_notify = SIGEV_NONE,
		.sigev_signo = 0xfacefeed,
		.sigev_value.sival_ptr =
			(void *) (unsigned long) 0xdeadbeefbadc0ded
	};

	if (syscall(__NR_timer_create, CLOCK_REALTIME, &sev, &tid[0]))
		return 77;
	printf("timer_create(CLOCK_REALTIME, {sigev_value={int=%d, ptr=%p}"
	       ", sigev_signo=%u, sigev_notify=SIGEV_NONE}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_signo, tid[0]);

	sev.sigev_notify = SIGEV_SIGNAL;
	sev.sigev_signo = SIGALRM;
	if (syscall(__NR_timer_create, CLOCK_MONOTONIC, &sev, &tid[1]))
		return 77;
	printf("timer_create(CLOCK_MONOTONIC, {sigev_value={int=%d, ptr=%p}"
	       ", sigev_signo=SIGALRM, sigev_notify=SIGEV_SIGNAL}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr, tid[1]);

	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function =
		(void *) (unsigned long) 0xdeadbeefbadc0ded;
	sev.sigev_notify_attributes =
		(void *) (unsigned long) 0xcafef00dfacefeed;
	if (syscall(__NR_timer_create, CLOCK_REALTIME, &sev, &tid[2]))
		return 77;
	printf("timer_create(CLOCK_REALTIME, {sigev_value={int=%d, ptr=%p}"
	       ", sigev_signo=SIGALRM, sigev_notify=SIGEV_THREAD"
	       ", sigev_notify_function=%p, sigev_notify_attributes=%p}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_notify_function,
	       sev.sigev_notify_attributes,
	       tid[2]);

#ifndef sigev_notify_thread_id
# if defined HAVE_STRUCT_SIGEVENT__SIGEV_UN__PAD
#  define sigev_notify_thread_id _sigev_un._pad[0]
# elif defined HAVE_STRUCT_SIGEVENT___PAD
#  define sigev_notify_thread_id __pad[0]
# endif
#endif /* !sigev_notify_thread_id */

#ifdef sigev_notify_thread_id
# ifndef SIGEV_THREAD_ID
#  define SIGEV_THREAD_ID 4
# endif
	sev.sigev_notify = SIGEV_THREAD_ID;
	sev.sigev_notify_thread_id = getpid();
	if (syscall(__NR_timer_create, CLOCK_MONOTONIC, &sev, &tid[3]))
		return 77;
	printf("timer_create(CLOCK_MONOTONIC, {sigev_value={int=%d, ptr=%p}"
	       ", sigev_signo=SIGALRM, sigev_notify=SIGEV_THREAD_ID"
	       ", sigev_notify_thread_id=%d}"
	       ", [%d]) = 0\n",
	       sev.sigev_value.sival_int,
	       sev.sigev_value.sival_ptr,
	       sev.sigev_notify_thread_id,
	       tid[3]);
#endif /* sigev_notify_thread_id */

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
