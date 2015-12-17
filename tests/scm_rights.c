/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/wait.h>

int main(int ac, const char **av)
{
	int i;
	int data = 0;
	struct iovec iov = {
		.iov_base = &data,
		.iov_len = sizeof(iov)
	};

	while ((i = open("/dev/null", O_RDWR)) < 3)
		assert(i >= 0);
	(void) close(3);

	int sv[2];
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == 0);
	int one = 1;
	assert(setsockopt(sv[0], SOL_SOCKET, SO_PASSCRED, &one, sizeof(one)) == 0);

	pid_t pid = fork();
	assert(pid >= 0);

	if (pid) {
		assert(close(sv[0]) == 0);
		assert(dup2(sv[1], 1) == 1);
		assert(close(sv[1]) == 0);

		int fds[ac];
		assert((fds[0] = open("/dev/null", O_RDWR)) == 3);
		for (i = 1; i < ac; ++i)
			assert((fds[i] = open(av[i], O_RDONLY)) == i + 3);

		union {
			struct cmsghdr cmsg;
			char buf[CMSG_LEN(sizeof(fds))];
		} control;

		control.cmsg.cmsg_level = SOL_SOCKET;
		control.cmsg.cmsg_type = SCM_RIGHTS;
		control.cmsg.cmsg_len = CMSG_LEN(sizeof(fds));
		memcpy(CMSG_DATA(&control.cmsg), fds, sizeof(fds));

		struct msghdr mh = {
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = &control,
			.msg_controllen = sizeof(control)
		};

		assert(sendmsg(1, &mh, 0) == sizeof(iov));
		assert(close(1) == 0);

                int status;
		assert(waitpid(pid, &status, 0) == pid);
		assert(status == 0);
	} else {
		assert(close(sv[1]) == 0);
		assert(dup2(sv[0], 0) == 0);
		assert(close(sv[0]) == 0);

		struct cmsghdr control[4 + ac * sizeof(int) / sizeof(struct cmsghdr)];

		struct msghdr mh = {
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = control,
			.msg_controllen = sizeof(control)
		};

		assert(recvmsg(0, &mh, 0) == sizeof(iov));
		assert(close(0) == 0);
	}

	return 0;
}
