/*
 * This file is part of attach-p-cmd strace test.
 *
 * Copyright (c) 2016-2017 Dmitry V. Levin <ldv@altlinux.org>
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
