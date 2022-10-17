/*
 * Check handling of CLONE_PTRACE'ed processes.
 *
 * Copyright (c) 2015-2022 The strace developers.
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

#ifndef QUIET_ATTACH
# define QUIET_ATTACH 0
#endif
#ifndef QUIET_EXIT
# define QUIET_EXIT 0
#endif

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

#ifdef IA64
extern int __clone2(int (*)(void *), void *, size_t, int, void *, ...);
# define do_clone(fn_, stack_, size_, flags_, arg_, ...)	\
	__clone2((fn_), (stack_), (size_), (flags_), (arg_), ## __VA_ARGS__)
#else
# define do_clone(fn_, stack_, size_, flags_, arg_, ...)	\
	clone((fn_), (stack_), (flags_), (arg_), ## __VA_ARGS__)
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

	const unsigned long child_stack_size = get_page_size();
	void *const child_stack =
		tail_alloc(child_stack_size * 2) + child_stack_size;

	const pid_t pid = do_clone(child, child_stack, child_stack_size,
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
#if !QUIET_ATTACH
	if (fprintf(fp, "%s: Detached unknown pid %d\n",
		    getenv("STRACE_EXE") ?: "strace", pid) < 0)
		perror_msg_and_fail("fprintf");
#endif

	int status;
	while (wait(&status) != pid) {
		if (errno != EINTR)
			perror_msg_and_fail("wait");
	}
	if (!WIFSIGNALED(status) || WTERMSIG(status) != sig)
		error_msg_and_fail("unexpected child exit status %d", status);

	char utm_str[64];
	char stm_str[64];
	printf("--- SIGCHLD {si_signo=SIGCHLD, si_code=CLD_KILLED, si_pid=%d"
	       ", si_uid=%d, si_status=%s, si_utime=%s, si_stime=%s} ---\n"
#if !QUIET_EXIT
	       "+++ exited with 0 +++\n"
#endif
	       , pid, geteuid(), "SIGUSR1",
	       clock_t_str((unsigned int) sinfo.si_utime, ARRSZ_PAIR(utm_str)),
	       clock_t_str((unsigned int) sinfo.si_stime, ARRSZ_PAIR(stm_str)));

	return 0;
}
