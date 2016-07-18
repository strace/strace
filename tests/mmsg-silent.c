/*
 * Check silent decoding of sendmmsg and recvmmsg syscalls.
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
#include <stdio.h>

#include "msghdr.h"

int
main(void)
{
	int fds[2];
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
		perror_msg_and_skip("socketpair");

	char buf = 'A';
	struct iovec iov = { .iov_base = &buf, .iov_len = sizeof(buf) };
	struct mmsghdr mh = {
		.msg_hdr = {
			.msg_iov = &iov,
			.msg_iovlen = 1
		}
	};

	int rc = send_mmsg(fds[1], &mh, 1, MSG_DONTWAIT);
	if (rc < 0)
		perror_msg_and_skip("sendmmsg");
	printf("sendmmsg(%d, %p, 1, MSG_DONTWAIT) = %d\n", fds[1], &mh, rc);

	struct timespec t = { .tv_sec = 0, .tv_nsec = 12345678 };
	rc = recv_mmsg(fds[0], &mh, 1, MSG_DONTWAIT, &t);
	printf("recvmmsg(%d, %p, 1, MSG_DONTWAIT, %p) = %d\n",
	       fds[0], &mh, &t, rc);

	puts("+++ exited with 0 +++");
	return 0;
}
