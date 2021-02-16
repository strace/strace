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
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "attach-p-cmd.h"

static void
write_pidfile(const pid_t pid)
{
	FILE *fp = fopen(pidfile, "w");
	if (!fp)
		perror_msg_and_fail("fopen: %s", pidfile);

	if (fprintf(fp, "%d", pid) < 0)
		perror_msg_and_fail("fprintf: %s", pidfile);

	if (fclose(fp))
		perror_msg_and_fail("fclose: %s", pidfile);
}

static void
wait_for_peer_invocation(void)
{
	/* wait for the lock directory to be created by peer */
	while (rmdir(lockdir)) {
		if (ENOENT != errno)
			perror_msg_and_fail("rmdir: %s", lockdir);
	}
}

int
main(void)
{
	const pid_t pid = getpid();
	write_pidfile(pid);

	wait_for_peer_invocation();

	static const char dir[] = "attach-p-cmd.test cmd";
	int rc = chdir(dir);

	printf("%-5d chdir(\"%s\") = %s\n"
	       "%-5d +++ exited with 0 +++\n",
	       pid, dir, sprintrc(rc), pid);

	return 0;
}
