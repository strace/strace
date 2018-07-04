/*
 * Check tracing of looping threads.
 *
 * Copyright (c) 2009-2018 The strace developers.
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

	/* Create a new process group.  */
	if (setpgid(0, 0))
		perror_msg_and_fail("setpgid");

	/*
	 * When the main process terminates, the process group becomes orphaned.
	 * If any member of the orphaned process group is stopped, then
	 * a SIGHUP signal followed by a SIGCONT signal is sent to each process
	 * in the orphaned process group.
	 * Create a process in a stopped state to activate this behaviour.
	 */
	pid_t stopped = fork();
	if (stopped < 0)
		perror_msg_and_fail("fork");
	if (!stopped) {
		raise(SIGSTOP);
		_exit(0);
	}

	const sigset_t set = {};
	const struct sigaction act = { .sa_handler = SIG_DFL };
	if (sigaction(SIGALRM, &act, NULL))
		perror_msg_and_fail("sigaction");
	if (sigprocmask(SIG_SETMASK, &set, NULL))
		perror_msg_and_fail("sigprocmask");
	alarm(timeout);

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
			if ((errno = pthread_create(&t, NULL, thread, NULL)))
				perror_msg_and_fail("pthread_create #%d", i);
		}

		/* This terminates all threads.  */
		_exit(0);
	}

	int s;
	if (waitpid(pid, &s, 0) != pid)
		perror_msg_and_fail("waitpid");

	assert(WIFEXITED(s));
	return WEXITSTATUS(s);
}
