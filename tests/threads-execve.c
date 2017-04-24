/*
 * Check decoding of threads when a non-leader thread invokes execve.
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
#include <asm/unistd.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

static pid_t leader;
static pid_t tid;

static void
handler(int signo)
{
}

static unsigned int sigsetsize;
static long
k_sigsuspend(const sigset_t *const set)
{
	return syscall(__NR_rt_sigsuspend, set, sigsetsize);
}

static pid_t
k_gettid(void)
{
	return syscall(__NR_gettid);
}

static void
get_sigsetsize(void)
{
	static const struct sigaction sa = { .sa_handler = handler };
	if (sigaction(SIGUSR1, &sa, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_BLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	raise(SIGUSR1);

	sigemptyset(&mask);
	for (sigsetsize = sizeof(mask) / sizeof(long);
	     sigsetsize; sigsetsize >>= 1) {
		long rc = k_sigsuspend(&mask);
		if (!rc)
			error_msg_and_fail("rt_sigsuspend");
		if (EINTR == errno)
			break;
		printf("%-5d rt_sigsuspend(%p, %u) = %s\n",
		       leader, &mask, sigsetsize, sprintrc(rc));
	}
	if (!sigsetsize)
		perror_msg_and_fail("rt_sigsuspend");
	printf("%-5d rt_sigsuspend([], %u) = ? ERESTARTNOHAND"
	       " (To be restarted if no handler)\n", leader, sigsetsize);
}

enum {
	ACTION_exit = 0,
	ACTION_rt_sigsuspend,
	ACTION_nanosleep,
	NUMBER_OF_ACTIONS
};

static const unsigned int NUMBER_OF_ITERATIONS = 1;
static unsigned int action;
static int fds[2];

static unsigned int
arglen(char **args)
{
	char **p;

	for (p = args; *p; ++p)
		;

	return p - args;
}

static void *
thread(void *arg)
{
	tid = k_gettid();

	static char buf[sizeof(action) * 3];
	sprintf(buf, "%u", action + 1);

	char **argv = arg;
	argv[2] = buf;

	if (read(fds[0], fds, sizeof(fds[0])))
		perror_msg_and_fail("execve");

	struct timespec ts = { .tv_nsec = 100000000 };
	(void) clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL);

	ts.tv_nsec = 12345;
	printf("%-5d nanosleep({tv_sec=0, tv_nsec=%u}, NULL) = 0\n",
	       tid, (unsigned int) ts.tv_nsec);

	switch (action % NUMBER_OF_ACTIONS) {
		case ACTION_exit:
			printf("%-5d execve(\"%s\", [\"%s\", \"%s\", \"%s\"]"
			       ", %p /* %u vars */ <pid changed to %u ...>\n",
			       tid, argv[0], argv[0], argv[1], argv[2],
			       environ, arglen(environ), leader);
			break;
		case ACTION_rt_sigsuspend:
			printf("%-5d execve(\"%s\", [\"%s\", \"%s\", \"%s\"]"
			       ", %p /* %u vars */ <unfinished ...>\n"
			       "%-5d <... rt_sigsuspend resumed>) = ?\n",
			       tid, argv[0], argv[0], argv[1], argv[2],
			       environ, arglen(environ),
			       leader);
			break;
		case ACTION_nanosleep:
			printf("%-5d execve(\"%s\", [\"%s\", \"%s\", \"%s\"]"
			       ", %p /* %u vars */ <unfinished ...>\n"
			       "%-5d <... nanosleep resumed> <unfinished ...>)"
			       " = ?\n",
			       tid, argv[0], argv[0], argv[1], argv[2],
			       environ, arglen(environ),
			       leader);
			break;
	}

	printf("%-5d +++ superseded by execve in pid %u +++\n"
	       "%-5d <... execve resumed> ) = 0\n",
	       leader, tid,
	       leader);

	(void) nanosleep(&ts, NULL);
	execve(argv[0], argv, environ);
	perror_msg_and_fail("execve");
}

int
main(int ac, char **av)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	leader = getpid();

	if (ac < 3) {
		struct timespec ts = { .tv_nsec = 1 };
		if (clock_nanosleep(CLOCK_REALTIME, 0, &ts, NULL))
			perror_msg_and_skip("clock_nanosleep CLOCK_REALTIME");

		get_sigsetsize();
		static char buf[sizeof(sigsetsize) * 3];
		sprintf(buf, "%u", sigsetsize);

		char *argv[] = { av[0], buf, (char *) "0", NULL };
		printf("%-5d execve(\"%s\", [\"%s\", \"%s\", \"%s\"]"
		       ", %p /* %u vars */) = 0\n",
		       leader, argv[0], argv[0], argv[1], argv[2],
		       environ, arglen(environ));
		execve(argv[0], argv, environ);
		perror_msg_and_fail("execve");
	}

	sigsetsize = atoi(av[1]);
	action = atoi(av[2]);

	if (action >= NUMBER_OF_ACTIONS * NUMBER_OF_ITERATIONS) {
		printf("%-5d +++ exited with 0 +++\n", leader);
		return 0;
	}

	if (pipe(fds))
		perror_msg_and_fail("pipe");

	pthread_t t;
	errno = pthread_create(&t, NULL, thread, av);
	if (errno)
		perror_msg_and_fail("pthread_create");

	struct timespec ts = { .tv_sec = 123 };
	sigset_t mask;
	sigemptyset(&mask);

	static char leader_str[sizeof(leader) * 3];
	int leader_str_len =
		snprintf(leader_str, sizeof(leader_str), "%-5d", leader);

	switch (action % NUMBER_OF_ACTIONS) {
		case ACTION_exit:
			printf("%s exit(42)%*s= ?\n", leader_str,
			       (int) sizeof(leader_str) - leader_str_len, " ");
			close(fds[1]);
			(void) syscall(__NR_exit, 42);
			break;
		case ACTION_rt_sigsuspend:
			printf("%s rt_sigsuspend([], %u <unfinished ...>\n",
			       leader_str, sigsetsize);
			close(fds[1]);
			(void) k_sigsuspend(&mask);
			break;
		case ACTION_nanosleep:
			printf("%s nanosleep({tv_sec=%u, tv_nsec=0}"
			       ",  <unfinished ...>\n",
			       leader_str, (unsigned int) ts.tv_sec);
			close(fds[1]);
			(void) nanosleep(&ts, 0);
			break;
	}

	return 1;
}
