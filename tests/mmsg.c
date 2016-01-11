/*
 * Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
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
# include <sys/syscall.h>

#if defined __NR_sendmmsg || defined HAVE_SENDMMSG

# include <assert.h>
# include <errno.h>
# include <fcntl.h>
# include <unistd.h>
# include <sys/socket.h>

#ifndef HAVE_STRUCT_MMSGHDR
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned msg_len;
};
#endif

static int
send_mmsg(int fd, struct mmsghdr *vec, unsigned int vlen, unsigned int flags)
{
	int rc;
#ifdef __NR_sendmmsg
	rc = syscall(__NR_sendmmsg, (long) fd, vec, (unsigned long) vlen,
		     (unsigned long) flags);
	if (rc >= 0 || ENOSYS != errno)
		return rc;
#endif
#ifdef HAVE_SENDMMSG
	rc = sendmmsg(fd, vec, vlen, flags);
#endif
	return rc;
}

static int
recv_mmsg(int fd, struct mmsghdr *vec, unsigned int vlen, unsigned int flags,
	  struct timespec *timeout)
{
	int rc;
#ifdef __NR_sendmmsg
	rc = syscall(__NR_recvmmsg, (long) fd, vec, (unsigned long) vlen,
		     (unsigned long) flags, timeout);
	if (rc >= 0 || ENOSYS != errno)
		return rc;
#endif
#ifdef HAVE_SENDMMSG
	rc = recvmmsg(fd, vec, vlen, flags, timeout);
#endif
	return rc;
}

int
main(void)
{
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
# define n_mmh (sizeof(mmh)/sizeof(mmh[0]))

	/*
	 * Following open/dup2/close calls make the output of strace
	 * more predictable, so we can just compare the output and
	 * expected output (mmsg.expected) for testing purposes.
	 */
	while ((fd = open("/dev/null", O_RDWR)) < 3)
		assert(fd >= 0);
	(void) close(3);

	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv))
		perror_msg_and_skip("socketpair");

	assert(dup2(sv[W], W) == W);
	assert(close(sv[W]) == 0);
	assert(dup2(sv[R], R) == R);
	assert(close(sv[R]) == 0);

	int r = send_mmsg(W, mmh, n_mmh, MSG_DONTROUTE | MSG_NOSIGNAL);
	if (r < 0 && errno == ENOSYS)
		perror_msg_and_skip("sendmmsg");
	assert((size_t)r == n_mmh);
	assert(close(W) == 0);

	assert(recv_mmsg(R, mmh, n_mmh, MSG_DONTWAIT, NULL) == n_mmh);
	assert(close(R) == 0);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sendmmsg || HAVE_SENDMMSG")

#endif
