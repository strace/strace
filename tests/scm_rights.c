/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>

int main(int ac, const char **av)
{
	assert(ac > 0);
	int fds[ac];

	static const char sample[] =
		"\xf1\xf2\xf3\xf4\xf5\xf6\xf7\xf8\xf9\xfa\xfb\xfc\xfd\xfe\xff";
	const unsigned int data_size = sizeof(sample) - 1;
	void *data = tail_alloc(data_size);
	memcpy(data, sample, data_size);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct iovec, iov);
	iov->iov_base = data;
	iov->iov_len = data_size;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct msghdr, mh);
	memset(mh, 0, sizeof(*mh));
	mh->msg_iov = iov;
	mh->msg_iovlen = 1;

	int i;
	while ((i = open("/dev/null", O_RDWR)) <= ac + 2)
		assert(i >= 0);
	while (i > 2)
		assert(close(i--) == 0);
	assert(close(0) == 0);

	int sv[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv))
		perror_msg_and_skip("socketpair");
	int one = 1;
	if (setsockopt(sv[0], SOL_SOCKET, SO_PASSCRED, &one, sizeof(one)))
		perror_msg_and_skip("setsockopt");

	assert((fds[0] = open("/dev/null", O_RDWR)) == 4);
	for (i = 1; i < ac; ++i)
		assert((fds[i] = open(av[i], O_RDONLY)) == i + 4);

	unsigned int cmsg_size = CMSG_SPACE(sizeof(fds));
	struct cmsghdr *cmsg = tail_alloc(cmsg_size);
	memset(cmsg, 0, cmsg_size);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(fds));
	memcpy(CMSG_DATA(cmsg), fds, sizeof(fds));

	mh->msg_control = cmsg;
	mh->msg_controllen = cmsg_size;

	assert(sendmsg(sv[1], mh, 0) == (int) data_size);

	assert(close(sv[1]) == 0);
	assert(open("/dev/null", O_RDWR) == sv[1]);

	for (i = 0; i < ac; ++i) {
		assert(close(fds[i]) == 0);
		fds[i] = 0;
	}

	cmsg_size += CMSG_SPACE(sizeof(struct ucred));
	cmsg = tail_alloc(cmsg_size);
	memset(cmsg, 0, cmsg_size);
	mh->msg_control = cmsg;
	mh->msg_controllen = cmsg_size;

	assert(recvmsg(0, mh, 0) == (int) data_size);
	assert(close(0) == 0);

	return 0;
}
