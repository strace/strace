/*
 * Check handling of CLONE_PTRACE'ed processes.
 *
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static siginfo_t sinfo;

static void
handler(const int no, siginfo_t *const si, void *const uc)
{
	memcpy(&sinfo, si, sizeof(sinfo));
}

static int
child(void *const arg)
{
	for(;;)
		pause();
	return 0;
}

#define child_stack_size	(get_page_size() / 2)

#ifdef IA64
extern int __clone2(int (*)(void *), void *, size_t, int, void *, ...);
# define clone(fn, child_stack, flags, arg)	\
		__clone2(fn, child_stack, child_stack_size, flags, arg)
#endif

int
main(void)
{
	const int sig = SIGUSR1;
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, sig);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	const pid_t pid = clone(child, tail_alloc(child_stack_size),
				CLONE_PTRACE | SIGCHLD, 0);
	if (pid < 0)
		perror_msg_and_fail("clone");

	static const struct sigaction sa = {
		.sa_sigaction = handler,
		.sa_flags = SA_SIGINFO
	};
	if (sigaction(SIGCHLD, &sa, NULL))
		perror_msg_and_fail("sigaction");

	kill(pid, sig);

	FILE *const fp = fdopen(3, "a");
	if (!fp)
		perror_msg_and_fail("fdopen");
	if (fprintf(fp, "%s: Detached unknown pid %d\n",
		    getenv("STRACE_EXE") ?: "strace", pid) < 0)
		perror_msg_and_fail("fprintf");

	int status;
	while (wait(&status) != pid) {
		if (errno != EINTR)
			perror_msg_and_fail("wait");
	}
	if (!WIFSIGNALED(status) || WTERMSIG(status) != sig)
		error_msg_and_fail("unexpected child exit status %d", status);

	printf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_KILLED, si_pid=%d"
	       ", si_uid=%u, si_status=%s, si_utime=%u, si_stime=%u} ---\n"
	       "+++ exited with 0 +++\n", pid, geteuid(), "SIGUSR1",
	       (unsigned int) sinfo.si_utime, (unsigned int) sinfo.si_stime);

	return 0;
}
