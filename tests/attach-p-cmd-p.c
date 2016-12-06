/*
 * This file is part of attach-p-cmd strace test.
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
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

static void
handler(int signo)
{
}

int
main(void)
{
	const struct sigaction act = { .sa_handler = handler };
	if (sigaction(SIGALRM, &act, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask = {};
	sigaddset(&mask, SIGALRM);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	static const char lockdir[] = "attach-p-cmd.test-lock";
	/* create a lock directory */
	if (mkdir(lockdir, 0700))
		perror_msg_and_fail("mkdir: %s", lockdir);

	/* wait for the lock directory to be removed by peer */
	while (mkdir(lockdir, 0700)) {
		if (EEXIST != errno)
			perror_msg_and_fail("mkdir: %s", lockdir);
	}

	/* remove the lock directory */
	if (rmdir(lockdir))
		perror_msg_and_fail("rmdir: %s", lockdir);

	alarm(1);
	pause();

	static const char dir[] = "attach-p-cmd.test -p";
	pid_t pid = getpid();
	int rc = chdir(dir);

	printf("%-5d --- SIGALRM {si_signo=SIGALRM, si_code=SI_KERNEL} ---\n"
	       "%-5d chdir(\"%s\") = %d %s (%m)\n"
	       "%-5d +++ exited with 0 +++\n",
	       pid, pid, dir, rc, errno2name(), pid);

	return 0;
}
