/*
 * Check decoding of NS_* commands of ioctl syscall.
 *
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <fcntl.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#include "nsfs.h"

#ifndef CLONE_NEWUSER
# define CLONE_NEWUSER 0x10000000
#endif

static void
test_no_namespace(void)
{
	ioctl(-1, NS_GET_USERNS);
	printf("ioctl(-1, NS_GET_USERNS) = -1 EBADF (%m)\n");
	ioctl(-1, NS_GET_PARENT);
	printf("ioctl(-1, NS_GET_PARENT) = -1 EBADF (%m)\n");
	ioctl(-1, NS_GET_NSTYPE);
	printf("ioctl(-1, NS_GET_NSTYPE) = -1 EBADF (%m)\n");
	ioctl(-1, NS_GET_OWNER_UID, NULL);
	printf("ioctl(-1, NS_GET_OWNER_UID, NULL) = -1 EBADF (%m)\n");
}

static void
test_clone(pid_t pid)
{
	char path[sizeof("/proc/%d/ns/user") + sizeof(int)*3];
	snprintf(path, sizeof(path), "/proc/%d/ns/user", pid);

	int ns_fd = open(path, O_RDONLY);
	if (ns_fd == -1)
		perror_msg_and_skip("open: %s", path);

	int userns_fd = ioctl(ns_fd, NS_GET_USERNS);
	printf("ioctl(%d, NS_GET_USERNS) = %s\n", ns_fd, sprintrc(userns_fd));

	int parent_ns_fd = ioctl(userns_fd, NS_GET_PARENT);
	printf("ioctl(%d, NS_GET_PARENT) = %s\n",
	       userns_fd, sprintrc(parent_ns_fd));

	int nstype = ioctl(userns_fd, NS_GET_NSTYPE);
	if (nstype == -1) {
		printf("ioctl(%d, NS_GET_NSTYPE) = %s\n",
		       userns_fd, sprintrc(nstype));
	} else {
		printf("ioctl(%d, NS_GET_NSTYPE) = %d (CLONE_NEWUSER)\n",
		       userns_fd, nstype);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, uid);
	int rc = ioctl(userns_fd, NS_GET_OWNER_UID, uid);
	if (rc == -1) {
		printf("ioctl(%d, NS_GET_OWNER_UID, %p) = %s\n",
		       userns_fd, uid, sprintrc(rc));
	} else {
		printf("ioctl(%d, NS_GET_OWNER_UID, [%u]) = %d\n",
		       userns_fd, *uid, rc);
	}
}

static int
child(void *arg)
{
	int *pipefd = (int *) arg;
	close(pipefd[1]);
	/* Wait for EOF from pipe. */
	if (read(pipefd[0], &pipefd[1], 1))
		perror_msg_and_fail("read");
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

static void
test_user_namespace(void)
{
	int pipefd[2];
	if (pipe(pipefd))
		perror_msg_and_fail("pipe");

	const unsigned long child_stack_size = get_page_size();
	void *const child_stack =
		tail_alloc(child_stack_size * 2) + child_stack_size;

	const pid_t pid = do_clone(child, child_stack, child_stack_size,
				   CLONE_NEWUSER | CLONE_UNTRACED | SIGCHLD,
				   pipefd);
	if (pid == -1) {
		perror("clone");
		return;
	}
	close(pipefd[0]);
	test_clone(pid);
	close(pipefd[1]);

	int status;
	if (wait(&status) != pid) {
		perror_msg_and_fail("wait");
	} else if (status != 0) {
		error_msg_and_fail("unexpected child exit status %d", status);
	}
}

int
main(void)
{
	test_no_namespace();
	test_user_namespace();
	puts("+++ exited with 0 +++");
	return 0;
}
