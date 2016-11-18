/*
 * Check how strace -e signal=set works.
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
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static pid_t pid;
static uid_t uid;

static void
handler(int sig)
{
}

static void
test_sig(int signo, const char *name)
{
	const struct sigaction act = { .sa_handler = handler };

	if (sigaction(signo, &act, NULL))
		perror_msg_and_fail("sigaction: %d", signo);

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, signo);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask: %d", signo);

	if (kill(pid, signo))
		perror_msg_and_fail("kill(%d, %d)", pid, signo);

	if (name && *name)
		printf("--- %s {si_signo=%s, si_code=SI_USER"
		       ", si_pid=%d, si_uid=%d} ---\n",
		       name, name, pid, uid);
}

int
main(int ac, const char **av)
{
	assert(ac & 1);

	pid = getpid();
	uid = geteuid();

	int i;
	for (i = 1; i < ac; i += 2)
		test_sig(atoi(av[i]), av[i + 1]);

	puts("+++ exited with 0 +++");
	return 0;
}
