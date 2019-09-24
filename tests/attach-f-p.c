/*
 * This file is part of attach-f-p strace test.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "scno.h"
#include <unistd.h>

#define N 3

typedef union {
	void *ptr;
	pid_t pid;
} retval_t;

static const char text_parent[] = "attach-f-p.test parent";
static const char *child[N] = {
	"attach-f-p.test child 0",
	"attach-f-p.test child 1",
	"attach-f-p.test child 2"
};
typedef int pipefd[2];
static pipefd pipes[N];

static void *
thread(void *a)
{
	unsigned int no = (long) a;
	int i;

	if (read(pipes[no][0], &i, sizeof(i)) != (int) sizeof(i))
		perror_msg_and_fail("read[%u]", no);
	assert(chdir(child[no]) == -1);
	retval_t retval = { .pid = syscall(__NR_gettid) };
	return retval.ptr;
}

int
main(void)
{
	pthread_t t[N];
	unsigned int i;

	if (write(1, "", 0) != 0)
		perror_msg_and_fail("write");

	for (i = 0; i < N; ++i) {
		if (pipe(pipes[i]))
			perror_msg_and_fail("pipe");

		errno = pthread_create(&t[i], NULL, thread, (void *) (long) i);
		if (errno)
			perror_msg_and_fail("pthread_create");
	}

	if (write(1, "\n", 1) != 1)
		perror_msg_and_fail("write");

	/* wait for the peer to write to stdout */
	struct stat st;
	for (;;) {
		if (fstat(1, &st))
			perror_msg_and_fail("fstat");
		if (st.st_size >= 103)
			break;
	}

	for (i = 0; i < N; ++i) {
		/* sleep a bit to let the tracer catch up */
		sleep(1);
		if (write(pipes[i][1], &i, sizeof(i)) != (int) sizeof(i))
			perror_msg_and_fail("write[%u]", i);
		retval_t retval;
		errno = pthread_join(t[i], &retval.ptr);
		if (errno)
			perror_msg_and_fail("pthread_join");
		errno = ENOENT;
		printf("%-5d chdir(\"%s\") = %s\n"
		       "%-5d +++ exited with 0 +++\n",
		       retval.pid, child[i], sprintrc(-1), retval.pid);
	}

	/* sleep a bit more to let the tracer catch up */
	sleep(1);

	pid_t pid = getpid();
	assert(chdir(text_parent) == -1);

	printf("%-5d chdir(\"%s\") = -1 ENOENT (%m)\n"
	       "%-5d +++ exited with 0 +++\n", pid, text_parent, pid);

	return 0;
}
