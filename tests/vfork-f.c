/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static inline int
logit_(const char *const str)
{
	return !chdir(str);
}

#define prefix "vfork-f."
#define logit(arg) logit_(prefix arg)

int main(int ac, char **av)
{
	if (ac < 1)
		return 1;
	if (ac > 1) {
		if (read(0, &ac, sizeof(int)))
			return 2;
		return logit("exec");
	}

	logit("start");

	int child_wait_fds[2];
	(void) close(0);
	if (pipe(child_wait_fds))
		perror_msg_and_fail("pipe");
	if (fcntl(child_wait_fds[1], F_SETFD, FD_CLOEXEC))
		perror_msg_and_fail("fcntl");

	int parent_wait_fds[2];
	if (pipe(parent_wait_fds))
		perror_msg_and_fail("pipe");
	if (fcntl(parent_wait_fds[0], F_SETFD, FD_CLOEXEC))
		perror_msg_and_fail("fcntl");
	if (fcntl(parent_wait_fds[1], F_SETFD, FD_CLOEXEC))
		perror_msg_and_fail("fcntl");

	char *const args[] = { av[0], (char *) "", NULL };
	pid_t pid = vfork();

	if (pid < 0)
		perror_msg_and_fail("vfork");

	if (!pid) {
		if (logit("child") || execve(args[0], args, args + 1))
			_exit(2);
	}

	close(0);
	close(parent_wait_fds[1]);

	if (read(parent_wait_fds[0], &parent_wait_fds[1], sizeof(int)))
		perror_msg_and_fail("read");
	logit("parent");
	close(child_wait_fds[1]);

	int status;
	assert(wait(&status) == pid);
	assert(status == 0);

	pid_t ppid = getpid();
	logit("finish");

	printf("%-5d chdir(\"%sstart\") = -1 ENOENT (%m)\n"
	       "%-5d chdir(\"%schild\") = -1 ENOENT (%m)\n"
	       "%-5d chdir(\"%sparent\") = -1 ENOENT (%m)\n"
	       "%-5d chdir(\"%sexec\") = -1 ENOENT (%m)\n"
	       "%-5d chdir(\"%sfinish\") = -1 ENOENT (%m)\n",
	       ppid, prefix,
	       pid, prefix,
	       ppid, prefix,
	       pid, prefix,
	       ppid, prefix);
	return 0;
}
