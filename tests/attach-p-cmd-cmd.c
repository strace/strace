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
