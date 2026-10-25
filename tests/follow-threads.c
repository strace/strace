/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <pthread.h>

static int
logit_(const char *const str)
{
	return !chdir(str);
}

#define prefix "follow-threads."
#define logit(arg) logit_(prefix arg)

static void *thread_call(void *arg)
{
	pid_t *tid = arg;
	*tid = syscall(SYS_gettid);
	logit("thread");
	return NULL;
}

int main(void)
{
	logit("start");

	pid_t pid = fork();

	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		logit("child");
		_exit(0);
	}

	logit("parent");

	pthread_t thread;

	pid_t tid;
	if (pthread_create(&thread, NULL, thread_call, &tid)) {
		perror_msg_and_fail("pthread_create");
	}
	pthread_join(thread, NULL);

	int status;
	assert(wait(&status) == pid);
	assert(status == 0);

	pid_t ppid = getpid();
	logit("finish");

	printf("%-5d chdir(\"%sstart\")" RVAL_ENOENT
		"%-5d chdir(\"%sparent\")" RVAL_ENOENT
		"%-5d chdir(\"%sthread\")" RVAL_ENOENT
		"%-5d chdir(\"%sfinish\")" RVAL_ENOENT,
		ppid, prefix,
		ppid, prefix,
		tid, prefix,
		ppid, prefix);
	return 0;
}