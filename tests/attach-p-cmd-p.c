/*
 * This file is part of attach-p-cmd strace test.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "attach-p-cmd.h"

static void
wait_for_peer_invocation(void)
{
	/* create the lock directory */
	if (mkdir(lockdir, 0700))
		perror_msg_and_fail("mkdir: %s", lockdir);

	/* wait for the lock directory to be removed by peer */
	while (mkdir(lockdir, 0700)) {
		if (EEXIST != errno)
			perror_msg_and_fail("mkdir: %s", lockdir);
	}

	/* cleanup the lock directory */
	if (rmdir(lockdir))
		perror_msg_and_fail("rmdir: %s", lockdir);
}

static void
wait_for_peer_termination(void)
{
	FILE *fp = fopen(pidfile, "r");
	if (!fp)
		perror_msg_and_fail("fopen: %s", pidfile);

	pid_t pid;
	if (fscanf(fp, "%d", &pid) < 0)
		perror_msg_and_fail("fscanf: %s", pidfile);
	if (pid < 0)
		error_msg_and_fail("pid = %d", pid);

	if (fclose(fp))
		perror_msg_and_fail("fclose: %s", pidfile);

	if (unlink(pidfile))
		perror_msg_and_fail("unlink: %s", pidfile);

	while (kill(pid, 0) == 0)
		;
}

int
main(void)
{
	wait_for_peer_invocation();
	wait_for_peer_termination();

	static const struct timespec ts = { .tv_nsec = 123456789 };
	if (nanosleep(&ts, NULL))
		perror_msg_and_fail("nanosleep");

	static const char dir[] = "attach-p-cmd.test -p";
	pid_t pid = getpid();
	int rc = chdir(dir);

	printf("%-5d chdir(\"%s\") = %s\n"
	       "%-5d +++ exited with 0 +++\n",
	       pid, dir, sprintrc(rc), pid);

	return 0;
}
