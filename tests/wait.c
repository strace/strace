/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <assert.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

int
main(void)
{
	int fds[2];
	int s;
	pid_t pid;
	struct rusage rusage = {};
	siginfo_t info = {};

	(void) close(0);
	(void) close(1);
	assert(!pipe(fds) && fds[0] == 0 && fds[1] == 1);

	pid = fork();
	assert(pid >= 0);
	if (!pid) {
		char c;
		(void) close(1);
		assert(read(0, &c, sizeof(c)) == 1);
		return 42;
	}

	(void) close(0);
	assert(wait4(pid, &s, WNOHANG | __WALL, NULL) == 0);
	assert(waitid(P_PID, pid, &info, WNOHANG | WEXITED) == 0);

	assert(write(1, "", 1) == 1);
	(void) close(1);
	assert(wait4(pid, &s, 0, &rusage) == pid);
	assert(WIFEXITED(s) && WEXITSTATUS(s) == 42);

	pid = fork();
	assert(pid >= 0);
	if (!pid) {
		(void) raise(SIGUSR1);
		return 77;
	}
	assert(wait4(pid, &s, __WALL, NULL) == pid);
	assert(WIFSIGNALED(s) && WTERMSIG(s) == SIGUSR1);

	pid = fork();
	assert(pid >= 0);
	if (!pid) {
		raise(SIGSTOP);
		return 0;
	}
	assert(wait4(pid, &s, WUNTRACED, NULL) == pid);
	assert(WIFSTOPPED(s) && WSTOPSIG(s) == SIGSTOP);

	assert(kill(pid, SIGCONT) == 0);
	assert(waitid(P_PID, pid, &info, WEXITED | WSTOPPED) == 0);
	assert(info.si_code == CLD_EXITED && info.si_status == 0);

	assert(wait4(-1, &s, WNOHANG | WUNTRACED | __WALL, &rusage) == -1);

	return 0;
}
