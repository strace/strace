/*
 * Check decoding of NS_* commands of ioctl syscall.
 *
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
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
# define clone(fn, child_stack, flags, arg)	\
		__clone2(fn, child_stack, get_page_size() / 2, flags, arg)
#endif

static void
test_user_namespace(void)
{
	pid_t pid;
	int pipefd[2];
	int status;

	if (pipe(pipefd))
		perror_msg_and_fail("pipe");

	pid = clone(child, tail_alloc(get_page_size() / 2),
		    CLONE_NEWUSER | CLONE_UNTRACED | SIGCHLD, pipefd);
	if (pid == -1) {
		perror("clone");
		return;
	}
	close(pipefd[0]);
	test_clone(pid);
	close(pipefd[1]);
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
