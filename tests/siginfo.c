/*
 * Check SIGCHLD siginfo_t decoding.
 *
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
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static siginfo_t sinfo;

static void
handler(int no, siginfo_t *si, void *uc)
{
	memcpy(&sinfo, si, sizeof(sinfo));
}

int
main(void)
{
	tprintf("%s", "");

	int fds[2];
	if (pipe(fds))
		perror_msg_and_fail("pipe");

	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		char c;
		(void) close(1);
		assert(read(0, &c, sizeof(c)) == 1);
		return 42;
	}

	(void) close(0);

	struct sigaction sa = {
		.sa_sigaction = handler,
		.sa_flags = SA_SIGINFO
	};
	assert(sigaction(SIGCHLD, &sa, NULL) == 0);

	sigset_t block_mask, unblock_mask;
	assert(sigprocmask(SIG_SETMASK, NULL, &block_mask) == 0);
	sigaddset(&block_mask, SIGCHLD);
	assert(sigprocmask(SIG_SETMASK, &block_mask, NULL) == 0);

	unblock_mask = block_mask;
	sigdelset(&unblock_mask, SIGCHLD);

	assert(write(1, "", 1) == 1);
	(void) close(1);

	sigsuspend(&unblock_mask);
	tprintf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED"
		", si_pid=%d, si_uid=%u, si_status=%d"
		", si_utime=%llu, si_stime=%llu} ---\n",
		sinfo.si_pid, sinfo.si_uid, sinfo.si_status,
		zero_extend_signed_to_ull(sinfo.si_utime),
		zero_extend_signed_to_ull(sinfo.si_stime));

	int s;
	assert(wait(&s) == pid);
	assert(WIFEXITED(s) && WEXITSTATUS(s) == 42);

	if (pipe(fds))
		perror_msg_and_fail("pipe");
	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		(void) close(1);
		char c;
		assert(read(0, &c, sizeof(c)) == 1);
		(void) raise(SIGUSR1);
		return 1;
	}

	(void) close(0);

	assert(write(1, "", 1) == 1);
	(void) close(1);

	sigsuspend(&unblock_mask);
	tprintf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_KILLED"
		", si_pid=%d, si_uid=%u, si_status=SIGUSR1"
		", si_utime=%llu, si_stime=%llu} ---\n",
		sinfo.si_pid, sinfo.si_uid,
		zero_extend_signed_to_ull(sinfo.si_utime),
		zero_extend_signed_to_ull(sinfo.si_stime));

	assert(wait(&s) == pid);
	assert(WIFSIGNALED(s) && WTERMSIG(s) == SIGUSR1);

	if (pipe(fds))
		perror_msg_and_fail("pipe");
	pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");

	if (!pid) {
		(void) close(1);
		raise(SIGSTOP);
		char c;
		assert(read(0, &c, sizeof(c)) == 1);
		return 0;
	}

	(void) close(0);

	sigsuspend(&unblock_mask);
	tprintf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_STOPPED"
		", si_pid=%d, si_uid=%u, si_status=SIGSTOP"
		", si_utime=%llu, si_stime=%llu} ---\n",
		sinfo.si_pid, sinfo.si_uid,
		zero_extend_signed_to_ull(sinfo.si_utime),
		zero_extend_signed_to_ull(sinfo.si_stime));

	assert(kill(pid, SIGCONT) == 0);

	sigsuspend(&unblock_mask);
	tprintf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_CONTINUED"
		", si_pid=%d, si_uid=%u, si_status=SIGCONT"
		", si_utime=%llu, si_stime=%llu} ---\n",
		sinfo.si_pid, sinfo.si_uid,
		zero_extend_signed_to_ull(sinfo.si_utime),
		zero_extend_signed_to_ull(sinfo.si_stime));

	assert(write(1, "", 1) == 1);
	(void) close(1);

	sigsuspend(&unblock_mask);
	tprintf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_EXITED"
		", si_pid=%d, si_uid=%u, si_status=0"
		", si_utime=%llu, si_stime=%llu} ---\n",
		sinfo.si_pid, sinfo.si_uid,
		zero_extend_signed_to_ull(sinfo.si_utime),
		zero_extend_signed_to_ull(sinfo.si_stime));

	assert(wait(&s) == pid && s == 0);

	tprintf("%s\n", "+++ exited with 0 +++");
	return 0;
}
