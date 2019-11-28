/*
 * Check tracing of looping threads.
 *
 * Copyright (c) 2009-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

static void *
thread(void *arg)
{
	for (;;)
		getuid();
	return arg;
}

int
main(int ac, const char *av[])
{
	assert(ac == 3);

	int timeout = atoi(av[1]);
	assert(timeout > 0);

	int num_threads = atoi(av[2]);
	assert(num_threads > 0);

	/*
	 * Unblock all signals.
	 */
	static sigset_t mask;
	if (sigprocmask(SIG_SETMASK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	/*
	 * Reset SIGALRM and SIGHUP signal handlers.
	 */
	static const struct sigaction sa_def = { .sa_handler = SIG_DFL };
	if (sigaction(SIGHUP, &sa_def, NULL))
		perror_msg_and_fail("sigaction SIGHUP");
	if (sigaction(SIGALRM, &sa_def, NULL))
		perror_msg_and_fail("sigaction SIGALRM");

	/*
	 * Create a new process group.
	 */
	if (setpgid(0, 0))
		perror_msg_and_fail("setpgid");

	/*
	 * Set an alarm clock.
	 */
	alarm(timeout);

	/*
	 * When the main process terminates, the process group becomes orphaned.
	 * If any member of the orphaned process group is stopped, then
	 * a SIGHUP signal followed by a SIGCONT signal is sent to each process
	 * in the orphaned process group.
	 * Create a process in a stopped state to activate this behaviour.
	 */
	const pid_t stopped = fork();
	if (stopped < 0)
		perror_msg_and_fail("fork");
	if (!stopped) {
		raise(SIGSTOP);
		_exit(0);
	}

	/*
	 * Wait for the process to stop.
	 */
	int status;
	if (waitpid(stopped, &status, WUNTRACED) != stopped)
		perror_msg_and_fail("waitpid WUNTRACED");
	if (!WIFSTOPPED(status) || WSTOPSIG(status) != SIGSTOP)
                error_msg_and_fail("waitpid WUNTRACED: "
				   "unexpected wait status %d", status);
	/*
	 * Create all threads in a subprocess, this guarantees that
	 * their tracer will not be their parent.
	 */
	pid_t pid = fork();
	if (pid < 0)
		perror_msg_and_fail("fork");
	if (!pid) {
		for (int i = 0; i < num_threads; i++) {
			pthread_t t;
			if ((errno = pthread_create(&t, NULL, thread, NULL))) {
				if (EAGAIN == errno)
					break;
				perror_msg_and_fail("pthread_create #%d", i);
			}
		}

		/* This terminates all threads created above.  */
		_exit(0);
	}

	if (waitpid(pid, &status, 0) != pid)
		perror_msg_and_fail("waitpid");
	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
                error_msg_and_fail("waitpid: unexpected wait status %d",
				   status);

	/*
	 * Make the process group orphaned.
	 */
	return 0;
}
