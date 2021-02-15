/*
 * Test PID namespace translation
 *
 * Copyright (c) 2020 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#ifdef __NR_fork

# include <errno.h>
# include <limits.h>
# include <sched.h>
# include <signal.h>
# include <stdio.h>
# include <stdlib.h>
# include <sys/wait.h>
# include <unistd.h>
# include <linux/sched.h>
# include "nsfs.h"

# ifndef CLONE_NEWUSER
#  define CLONE_NEWUSER 0x10000000
# endif

# ifndef CLONE_NEWPID
#  define CLONE_NEWPID 0x20000000
# endif

static int
fork_chain(int depth)
{
	if (!depth)
		return 0;

	int pid = syscall(__NR_fork);
	if (pid < 0)
		return errno;

	if (!pid)
		_exit(fork_chain(depth - 1));

	int status;
	if (waitpid(pid, &status, 0) < 0) {
		if (errno == ECHILD)
			_exit(fork_chain(depth - 1));
		return errno;
	}

	if (!WIFEXITED(status))
		return -1;

	return WEXITSTATUS(status);
}

int main(void)
{
	check_ns_ioctl();

	if (unshare(CLONE_NEWPID | CLONE_NEWUSER) < 0) {
		if (errno == EPERM)
			perror_msg_and_skip("unshare");

		perror_msg_and_fail("unshare");
	}

	errno = fork_chain(2);
	if (errno)
		perror_msg_and_fail("fork_chain");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fork")

#endif
