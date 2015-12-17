/*
 * Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>

int
main(void)
{
#if defined(HAVE_SENDMMSG) && defined(HAVE_STRUCT_MMSGHDR)
	const int R = 0, W = 1;
	int fd;
	int sv[2];
	char one[] = "one";
	char two[] = "two";
	char three[] = "three";

	struct iovec iov[] = {
		{
			.iov_base = one,
			.iov_len = sizeof(one) - 1
		}, {
			.iov_base = two,
			.iov_len = sizeof(two) - 1
		}, {
			.iov_base = three,
			.iov_len = sizeof(three) - 1
		}
	};

	struct mmsghdr mmh[] = {
		{
			.msg_hdr = {
				.msg_iov = iov + 0,
				.msg_iovlen = 2,
			}
		}, {
			.msg_hdr = {
				.msg_iov = iov + 2,
				.msg_iovlen = 1,
			}
		}
	};
#define n_mmh (sizeof(mmh)/sizeof(mmh[0]))

	/*
	 * Following open/dup2/close calls make the output of strace
	 * more predictable, so we can just compare the output and
	 * expected output (mmsg.expected) for testing purposes.
	 */
	while ((fd = open("/dev/null", O_RDWR)) < 3)
		assert(fd >= 0);
	(void) close(3);

	assert(socketpair(AF_UNIX, SOCK_DGRAM, 0, sv) == 0);

	assert(dup2(sv[W], W) == W);
	assert(close(sv[W]) == 0);
	assert(dup2(sv[R], R) == R);
	assert(close(sv[R]) == 0);

	int r = sendmmsg(W, mmh, n_mmh, 0);
	if (r < 0 && errno == ENOSYS)
		return 77;
	assert((size_t)r == n_mmh);
	assert(close(W) == 0);

	assert(recvmmsg(R, mmh, n_mmh, 0, NULL) == n_mmh);
	assert(close(R) == 0);

	return 0;
#else
	return 77;
#endif
}
