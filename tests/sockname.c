/*
 * Check decoding of sockname family syscalls.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "secontext.h"

#ifndef TEST_SYSCALL_NAME
# error TEST_SYSCALL_NAME must be defined
#endif

#ifndef TEST_SYSCALL_STR
# define TEST_SYSCALL_STR STRINGIFY_VAL(TEST_SYSCALL_NAME)
#endif
#define TEST_SOCKET TEST_SYSCALL_STR ".socket"

#ifdef TEST_SYSCALL_PREPARE
# define PREPARE_TEST_SYSCALL_INVOCATION do { TEST_SYSCALL_PREPARE; } while (0)
#else
# define PREPARE_TEST_SYSCALL_INVOCATION do {} while (0)
#endif

#ifndef PREFIX_S_ARGS
# define PREFIX_S_ARGS
#endif
#ifndef PREFIX_F_ARGS
# define PREFIX_F_ARGS
#endif
#ifndef PREFIX_S_STR
# define PREFIX_S_STR ""
#endif
#ifndef PREFIX_F_STR
# define PREFIX_F_STR ""
#endif
#ifndef SUFFIX_ARGS
# define SUFFIX_ARGS
#endif
#ifndef SUFFIX_STR
# define SUFFIX_STR ""
#endif

static void
test_sockname_syscall(const int fd)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, plen);
	*plen = sizeof(struct sockaddr_un);
	struct sockaddr_un *addr = tail_alloc(*plen);

	char *my_secontext = SECONTEXT_PID_MY();
	char *fd_secontext = SECONTEXT_FD(fd);

	PREPARE_TEST_SYSCALL_INVOCATION;
	int rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
				   plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s%s(%d%s%s, {sa_family=AF_UNIX, sun_path=\"%s\"%s}"
	       ", [%d => %d]%s) = %d\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_S_STR,
	       addr->sun_path, SECONTEXT_FILE(addr->sun_path),
	       (int) sizeof(struct sockaddr_un), (int) *plen, SUFFIX_STR, rc);

	memset(addr, 0, sizeof(*addr));
	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s%s(%d%s%s, {sa_family=AF_UNIX, sun_path=\"%s\"%s}"
	       ", [%d]%s) = %d\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_S_STR,
	       addr->sun_path, SECONTEXT_FILE(addr->sun_path),
	       (int) *plen, SUFFIX_STR, rc);

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_F_ARGS, (void *) addr, 0 SUFFIX_ARGS);
	printf("%s%s(%d%s%s, %p, NULL%s) = %s\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_F_STR,
	       addr, SUFFIX_STR, sprintrc(rc));

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, 0, 0 SUFFIX_ARGS);
	printf("%s%s(%d%s%s, NULL, NULL%s) = %s\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext,
	       rc == -1 ? PREFIX_F_STR : PREFIX_S_STR,
	       SUFFIX_STR, sprintrc(rc));

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_F_ARGS, (void *) addr,
			       plen + 1 SUFFIX_ARGS);
	printf("%s%s(%d%s%s, %p, %p%s) = %s\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_F_STR, addr,
	       plen + 1, SUFFIX_STR, sprintrc(rc));

	const size_t offsetof_sun_path = offsetof(struct sockaddr_un, sun_path);
	*plen = offsetof_sun_path;
	memset(addr->sun_path, 'A', sizeof(addr->sun_path));

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s%s(%d%s%s, {sa_family=AF_UNIX}, [%d => %d]%s) = %d\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_S_STR,
	       (int) offsetof_sun_path, (int) *plen, SUFFIX_STR, rc);

	++addr;
	*plen = sizeof(struct sockaddr);
	addr = (void *) addr - *plen;

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s%s(%d%s%s, {sa_family=AF_UNIX, sun_path=\"%.*s\"%s}"
	       ", [%d => %d]%s) = %d\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_S_STR,
	       (int) (sizeof(struct sockaddr) - offsetof_sun_path),
	       addr->sun_path, SECONTEXT_FILE(addr->sun_path),
	       (int) sizeof(struct sockaddr), (int) *plen, SUFFIX_STR, rc);

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_F_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	printf("%s%s(%d%s%s, %p, [%d]%s) = %s\n",
	       my_secontext,
	       TEST_SYSCALL_STR, fd, fd_secontext, PREFIX_F_STR, addr,
	       *plen, SUFFIX_STR, sprintrc(rc));
}
