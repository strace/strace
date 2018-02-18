/*
 * Check delay injection.
 *
 * Copyright (c) 2018 The strace developers.
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
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <asm/unistd.h>

static int64_t
usecs_from_tv(const struct timeval *const tv)
{
    return (int64_t) tv->tv_sec * 1000000 + tv->tv_usec;
}

static int64_t
usecs_from_ts(const struct timespec *const ts)
{
    return (int64_t) ts->tv_sec * 1000000 + ts->tv_nsec / 1000;
}

static void
check_delay(const struct timeval *const tv0,
	    const struct timespec *const ts,
	    const struct timeval *const tv1,
	    const int nproc,
	    const int64_t delay_enter,
	    const int64_t delay_exit)
{
	const int64_t us0 = usecs_from_tv(tv0);
	const int64_t us  = usecs_from_ts(ts);
	const int64_t us1 = usecs_from_tv(tv1);

	if (us - us0 < delay_exit * (nproc - 1) / nproc)
		_exit(1);

	if (us - us0 > delay_exit * (nproc + 1) / nproc)
		_exit(2);

	if (us1 - us < delay_enter * (nproc - 1) / nproc)
		_exit(3);

	if (us1 - us > delay_enter * (nproc + 1) / nproc)
		_exit(4);
}

static void
run(const int nproc, const int delay_enter, const int delay_exit)
{
	struct timeval prev = { 0, 0 }, now;

	for (int i = 0; i < nproc; ++i, prev = now) {
		struct timespec ts;

		if (i && clock_gettime(CLOCK_REALTIME, &ts))
			perror_msg_and_fail("clock_gettime");

		if (syscall(__NR_gettimeofday, &now, NULL))
			perror_msg_and_fail("gettimeofday");

		if (!i)
			continue;

		check_delay(&prev, &ts, &now, nproc, delay_enter, delay_exit);
	}

	_exit(0);
}

int
main(int ac, char *av[])
{
	if (ac != 4)
		error_msg_and_fail("usage: delay <nproc> <delay_enter> <delay_exit>");

	const int nproc = atoi(av[1]);
	if (nproc <= 1)
		perror_msg_and_fail("invalid nproc: %s", av[1]);

	const int delay_enter = atoi(av[2]);
	if (delay_enter <= 0)
		perror_msg_and_fail("invalid delay_enter: %s", av[2]);

	const int delay_exit = atoi(av[3]);
	if (delay_exit <= 0)
		perror_msg_and_fail("invalid delay_exit: %s", av[3]);

	for (int i = 0; i < nproc; ++i) {
		pid_t pid = fork();

		if (pid)
			usleep(MAX(delay_enter, delay_exit) / nproc);
		else
			run(nproc, delay_enter, delay_exit);
	}

	int status;
	while (wait(&status) > 0) {
		if (status)
			perror_msg_and_fail("wait status %d", status);
	}

	return 0;
}
