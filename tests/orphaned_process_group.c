/*
 * Check tracing of orphaned process group.
 *
 * Copyright (c) 2019-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define TIMEOUT 5

static void
alarm_handler(const int no)
{
	error_msg_and_skip("Orphaned process group semantics"
			   " is not supported by the kernel");
}

int
main(void)
{
	int status;

	/*
	 * Unblock all signals.
	 */
	static sigset_t mask;
	if (sigprocmask(SIG_SETMASK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	/*
	 * Create a pipe to track termination of processes.
	 */
	int pipe_fds[2];
	if (pipe(pipe_fds))
		perror_msg_and_fail("pipe");

	/*
	 * Create a leader for its own new process group.
	 */
	pid_t leader = fork();
	if (leader < 0)
		perror_msg_and_fail("fork");

	if (leader) {
		/*
		 * Close the writing end of the pipe.
		 */
		close(pipe_fds[1]);

		/*
		 * Install the SIGALRM signal handler.
		 */
		static const struct sigaction sa = {
			.sa_handler = alarm_handler
		};
		if (sigaction(SIGALRM, &sa, NULL))
			perror_msg_and_fail("sigaction");

		/*
		 * Set an alarm clock.
		 */
		alarm(TIMEOUT);

		/*
		 * Wait for termination of the child process.
		 */
		if (waitpid(leader, &status, 0) != leader)
			perror_msg_and_fail("waitpid leader");
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
			error_msg_and_fail("waitpid leader: "
					   "unexpected wait status %d",
					   status);

		/*
		 * Wait for termination of all processes
		 * in the process group of the child process.
		 */
		if (read(pipe_fds[0], &status, sizeof(status)) != 0)
			perror_msg_and_fail("read");

		/*
		 * At this point all processes are gone.
		 * Let the tracer time to catch up.
		 */
		alarm(0);
		sleep(1);
		return 0;
	}

	/*
	 * Close the reading end of the pipe.
	 */
	close(pipe_fds[0]);

	/*
	 * Create a new process group.
	 */
	if (setpgid(0, 0))
		perror_msg_and_fail("setpgid");

	/*
	 * When the leader process terminates, the process group becomes orphaned.
	 * If any member of the orphaned process group is stopped, then
	 * a SIGHUP signal followed by a SIGCONT signal is sent to each process
	 * in the orphaned process group.
	 * Create a process in a stopped state to activate this behaviour.
	 */
	const pid_t stopped = fork();
	if (stopped < 0)
		perror_msg_and_fail("fork");
	if (!stopped) {
		static const struct sigaction sa = { .sa_handler = SIG_DFL };
		if (sigaction(SIGHUP, &sa, NULL))
			perror_msg_and_fail("sigaction");

		raise(SIGSTOP);
		_exit(0);
	}

	/*
	 * Wait for the process to stop.
	 */
	if (waitpid(stopped, &status, WUNTRACED) != stopped)
		perror_msg_and_fail("waitpid WUNTRACED");
	if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP)
                error_msg_and_fail("unexpected wait status %d", status);

	/*
	 * Print the expected output.
	 */
	leader = getpid();
	printf("%-5d --- %s {si_signo=%s, si_code=SI_TKILL"
	       ", si_pid=%d, si_uid=%d} ---\n",
	       stopped, "SIGSTOP", "SIGSTOP", stopped, geteuid());
	printf("%-5d --- stopped by SIGSTOP ---\n", stopped);
	printf("%-5d +++ exited with 0 +++\n", leader);
	printf("%-5d --- %s {si_signo=%s, si_code=SI_KERNEL} ---\n",
	       stopped, "SIGHUP", "SIGHUP");
	printf("%-5d +++ killed by %s +++\n", stopped, "SIGHUP");
	printf("%-5d +++ exited with 0 +++\n", getppid());

	/*
	 * Make the process group orphaned.
	 */
	return 0;
}
