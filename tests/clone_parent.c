/*
 * Check handling of CLONE_PARENT'ed processes.
 *
 * Copyright (c) 2017-2020 The strace developers.
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
#include <sys/wait.h>
#include <unistd.h>

#ifndef QUIET_MSG
# define QUIET_MSG 0
#endif

static int
child(void *const arg)
{
	return 42;
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
	const unsigned long child_stack_size = get_page_size();
	void *const child_stack =
		tail_alloc(child_stack_size * 2) + child_stack_size;

	const pid_t pid = do_clone(child, child_stack, child_stack_size,
				   CLONE_PARENT | SIGCHLD, 0);
	if (pid < 0)
		perror_msg_and_fail("clone");

	int status;
	if (wait(&status) >= 0)
		error_msg_and_fail("unexpected return code from wait");

	while (!kill(pid, 0))
		;
	if (errno != ESRCH)
		perror_msg_and_fail("kill");

	FILE *const fp = fdopen(3, "a");
	if (!fp)
		perror_msg_and_fail("fdopen");
#if !QUIET_MSG
	if (fprintf(fp, "%s: Exit of unknown pid %d ignored\n",
		    getenv("STRACE_EXE") ?: "strace", pid) < 0)
		perror_msg_and_fail("fprintf");
#endif

#if !QUIET_MSG
	puts("+++ exited with 0 +++");
#endif
	return 0;
}
