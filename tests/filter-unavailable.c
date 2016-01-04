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
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/wait.h>

#define P 16
#define T 7

static void *
thread(void *arg)
{
	assert(write(1, "", 1) == 1);
	pause();
	return arg;
}

static int
process(void)
{
	int i;
	int fds[2];
	pthread_t t;
	struct timespec ts = { .tv_nsec = 10000000 };

	(void) close(0);
	(void) close(1);
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	for (i = 0; i < T; ++i)
		assert(pthread_create(&t, NULL, thread, NULL) == 0);
	for (i = 0; i < T; ++i)
		assert(read(0, fds, 1) == 1);

	(void) nanosleep(&ts, 0);
	return 0;
}

int
main(void)
{
	int i, s;
	pid_t p;

	for (i = 0; i < P; ++i) {
		p = fork();
		if (p < 0)
			perror_msg_and_fail("fork");
		if (p == 0)
			return process();
		assert(waitpid(p, &s, 0) == p);
		assert(WIFEXITED(s));
		if (WEXITSTATUS(s))
			return WEXITSTATUS(s);
	}
	return 0;
}
