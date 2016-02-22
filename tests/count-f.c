/*
 * This file is part of count-f strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 32
#define P 8
#define T 4

static void *
thread(void *arg)
{
	unsigned int i;

	assert(chdir(".") == 0);
	for (i = 0; i < N; ++i) {
		assert(chdir("") == -1);
		assert(chdir(".") == 0);
	}

	return NULL;
}

static int
process(void)
{
	unsigned int i;
	pthread_t t[T];

	for (i = 0; i < T; ++i) {
		errno = pthread_create(&t[i], NULL, thread, NULL);
		if (errno)
			perror_msg_and_fail("pthread_create");
	}

	for (i = 0; i < T; ++i) {
		void *retval;
		errno = pthread_join(t[i], &retval);
		if (errno)
			perror_msg_and_fail("pthread_join");
	}

	return 0;
}

int
main(void)
{
	unsigned int i;
	pid_t p[P];

	for (i = 0; i < P; ++i) {
		p[i] = fork();
		if (p[i] < 0)
			perror_msg_and_fail("fork");
		if (!p[i])
			return process();
	}
	for (i = 0; i < P; ++i) {
		int s;

		assert(waitpid(p[i], &s, 0) == p[i]);
		assert(WIFEXITED(s));
		if (WEXITSTATUS(s))
			return WEXITSTATUS(s);
	}

	return 0;
}
