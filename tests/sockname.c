/*
 * Check decoding of sockname family syscalls.
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/un.h>

#ifndef TEST_SYSCALL_NAME
# error TEST_SYSCALL_NAME must be defined
#endif

#define TEST_SYSCALL_STR__(a) #a
#define TEST_SYSCALL_STR_(a) TEST_SYSCALL_STR__(a)
#define TEST_SYSCALL_STR TEST_SYSCALL_STR_(TEST_SYSCALL_NAME)
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

	PREPARE_TEST_SYSCALL_INVOCATION;
	int rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
				   plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s(%d%s, {sa_family=AF_UNIX, sun_path=\"%s\"}"
	       ", [%d->%d]%s) = %d\n",
	       TEST_SYSCALL_STR, fd, PREFIX_S_STR, addr->sun_path,
	       (int) sizeof(struct sockaddr_un), (int) *plen, SUFFIX_STR, rc);

	memset(addr, 0, sizeof(*addr));
	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s(%d%s, {sa_family=AF_UNIX, sun_path=\"%s\"}"
	       ", [%d]%s) = %d\n",
	       TEST_SYSCALL_STR, fd, PREFIX_S_STR, addr->sun_path,
	       (int) *plen, SUFFIX_STR, rc);

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_F_ARGS, (void *) addr, 0 SUFFIX_ARGS);
	printf("%s(%d%s, %p, NULL%s) = %s\n",
	       TEST_SYSCALL_STR, fd, PREFIX_F_STR, addr, SUFFIX_STR,
	       sprintrc(rc));

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, 0, 0 SUFFIX_ARGS);
	printf("%s(%d%s, NULL, NULL%s) = %s\n",
	       TEST_SYSCALL_STR, fd, rc == -1 ? PREFIX_F_STR : PREFIX_S_STR,
	       SUFFIX_STR, sprintrc(rc));

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_F_ARGS, (void *) addr,
			       plen + 1 SUFFIX_ARGS);
	printf("%s(%d%s, %p, %p%s) = %s\n",
	       TEST_SYSCALL_STR, fd, PREFIX_F_STR, addr,
	       plen + 1, SUFFIX_STR, sprintrc(rc));

	const size_t offsetof_sun_path = offsetof(struct sockaddr_un, sun_path);
	*plen = offsetof_sun_path;
	memset(addr->sun_path, 'A', sizeof(addr->sun_path));

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s(%d%s, {sa_family=AF_UNIX}, [%d->%d]%s) = %d\n",
	       TEST_SYSCALL_STR, fd, PREFIX_S_STR,
	       (int) offsetof_sun_path, (int) *plen, SUFFIX_STR, rc);

	++addr;
	*plen = sizeof(struct sockaddr);
	addr = (void *) addr - *plen;

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_S_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	if (rc < 0)
		perror_msg_and_skip(TEST_SYSCALL_STR);
	printf("%s(%d%s, {sa_family=AF_UNIX, sun_path=\"%.*s\"}"
	       ", [%d->%d]%s) = %d\n",
	       TEST_SYSCALL_STR, fd, PREFIX_S_STR,
	       (int) (sizeof(struct sockaddr) - offsetof_sun_path),
	       addr->sun_path, (int) sizeof(struct sockaddr),
	       (int) *plen, SUFFIX_STR, rc);

	PREPARE_TEST_SYSCALL_INVOCATION;
	rc = TEST_SYSCALL_NAME(fd PREFIX_F_ARGS, (void *) addr,
			       plen SUFFIX_ARGS);
	printf("%s(%d%s, %p, [%d]%s) = %s\n",
	       TEST_SYSCALL_STR, fd, PREFIX_F_STR, addr,
	       *plen, SUFFIX_STR, sprintrc(rc));
}
