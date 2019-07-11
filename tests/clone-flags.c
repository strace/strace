/*
 * Check decoding of clone flags.
 *
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <limits.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

#define XLAT_MACROS_ONLY
#include "xlat/clone_flags.h"
#undef XLAT_MACROS_ONLY

static const int child_exit_status = 42;
static pid_t pid;

static pid_t
wait_cloned(pid_t pid, int sig, const char *text)
{
	if (pid < 0)
		perror_msg_and_fail("clone %s", text);
	int status;
	pid_t rc = wait(&status);
	if (sig) {
		if (rc != pid)
			perror_msg_and_fail("unexpected wait rc %d from %s",
					    rc, text);
		if (!WIFEXITED(status) ||
		    WEXITSTATUS(status) != child_exit_status)
			error_msg_and_fail("unexpected wait status %#x from %s",
					   status, text);
	} else {
		if (rc >= 0)
			error_msg_and_fail("unexpected wait rc %d from %s",
					   rc, text);
	}
	return pid;
}

#ifdef IA64
extern int __clone2(int (*)(void *), void *, size_t, int, void *, ...);
# define do_clone(fn_, stack_, size_, flags_, arg_, ...) \
	wait_cloned(__clone2((fn_), (stack_), (size_), (flags_), (arg_), \
			     ## __VA_ARGS__), (flags_) & 0xff, #flags_)
# define SYSCALL_NAME "clone2"
# define STACK_SIZE_FMT ", stack_size=%#lx"
# define STACK_SIZE_ARG child_stack_size,
#else
# define do_clone(fn_, stack_, size_, flags_, arg_, ...) \
	wait_cloned(clone((fn_), (stack_), (flags_), (arg_),	\
			  ## __VA_ARGS__), (flags_) & 0xff, #flags_)
# define SYSCALL_NAME "clone"
# define STACK_SIZE_FMT ""
# define STACK_SIZE_ARG
#endif

static int
child(void *const arg)
{
	return child_exit_status;
}

int
main(void)
{
	const unsigned long child_stack_size = get_page_size();
	void *const child_stack =
		tail_alloc(child_stack_size * 2) + child_stack_size;

	const char *child_stack_expected_str = getenv("CHILD_STACK_EXPECTED");
	const char *child_stack_reported_str = getenv("CHILD_STACK_REPORTED");

	if (!child_stack_expected_str || !child_stack_reported_str) {
		const unsigned long child_stack_base =
			(unsigned long) child_stack / 0x1000;
		pid = do_clone(child, child_stack, child_stack_size,
			       SIGCHLD, 0);
		printf("%s\\(child_stack=(%#lx|%#lx)[[:xdigit:]]{3}"
		       STACK_SIZE_FMT ", flags=%s\\) = %d\n",
		       SYSCALL_NAME, child_stack_base, child_stack_base - 1,
		       STACK_SIZE_ARG "SIGCHLD", pid);
		return 0;
	}
	const unsigned long child_stack_expected =
		strtoul(child_stack_expected_str, 0, 16);
	const unsigned long child_stack_reported =
		strtoul(child_stack_reported_str, 0, 16);
	const unsigned long child_stack_printed =
		(unsigned long) child_stack +
		(child_stack_reported - child_stack_expected);

	pid = do_clone(child, child_stack, child_stack_size, 0, 0);
	printf("%s(child_stack=%#lx" STACK_SIZE_FMT ", flags=%s) = %d\n",
	       SYSCALL_NAME, child_stack_printed, STACK_SIZE_ARG
	       "0", pid);

	pid = do_clone(child, child_stack, child_stack_size, CLONE_FS, 0);
	printf("%s(child_stack=%#lx" STACK_SIZE_FMT ", flags=%s) = %d\n",
	       SYSCALL_NAME, child_stack_printed, STACK_SIZE_ARG
	       "CLONE_FS", pid);

	pid = do_clone(child, child_stack, child_stack_size, SIGCHLD, 0);
	printf("%s(child_stack=%#lx" STACK_SIZE_FMT ", flags=%s) = %d\n",
	       SYSCALL_NAME, child_stack_printed, STACK_SIZE_ARG
	       "SIGCHLD", pid);

	pid = do_clone(child, child_stack, child_stack_size,
		       CLONE_FS|SIGCHLD, 0);
	printf("%s(child_stack=%#lx" STACK_SIZE_FMT ", flags=%s) = %d\n",
	       SYSCALL_NAME, child_stack_printed, STACK_SIZE_ARG
	       "CLONE_FS|SIGCHLD", pid);

	TAIL_ALLOC_OBJECT_CONST_PTR(pid_t, ptid);
	pid = do_clone(child, child_stack, child_stack_size,
		       CLONE_PARENT_SETTID|SIGCHLD, 0, ptid);
	printf("%s(child_stack=%#lx" STACK_SIZE_FMT ", flags=%s"
	       ", parent_tid=[%u]) = %d\n",
	       SYSCALL_NAME, child_stack_printed, STACK_SIZE_ARG
	       "CLONE_PARENT_SETTID|SIGCHLD", *ptid, pid);

	char buf[PATH_MAX];
	if (readlink("/proc/self/fd/0", buf, sizeof(buf) - 1) > 0) {
		*ptid = 0;
		pid = do_clone(child, child_stack, child_stack_size,
			       CLONE_PIDFD|SIGCHLD, 0, ptid);
		char *fname = 0;
		if (asprintf(&fname, "/proc/self/fd/%d", *ptid) < 0)
			perror_msg_and_fail("asprintf");
		int rc = readlink(fname, buf, sizeof(buf) - 1);
		if ((unsigned int) rc >= sizeof(buf))
			perror_msg_and_fail("readlink");
		buf[rc] = '\0';
		printf("%s(child_stack=%#lx" STACK_SIZE_FMT ", flags=%s"
		       ", parent_tid=[%u<%s>]) = %d\n",
		       SYSCALL_NAME, child_stack_printed, STACK_SIZE_ARG
		       "CLONE_PIDFD|SIGCHLD", *ptid, buf, pid);
	}

	return 0;
}
