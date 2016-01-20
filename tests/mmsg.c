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

#if (defined __NR_sendmmsg || defined HAVE_SENDMMSG) \
 && (defined __NR_recvmmsg || defined HAVE_RECVMMSG)

# include <assert.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/socket.h>

# ifndef HAVE_STRUCT_MMSGHDR
struct mmsghdr {
	struct msghdr msg_hdr;
	unsigned msg_len;
};
# endif

# define LENGTH_OF(arg) ((unsigned int) sizeof(arg) - 1)

static int
send_mmsg(int fd, struct mmsghdr *vec, unsigned int vlen, unsigned int flags)
{
	int rc;
#ifdef __NR_sendmmsg
	rc = syscall(__NR_sendmmsg, (long) fd, vec, (unsigned long) vlen,
		     (unsigned long) flags);
	if (rc >= 0 || ENOSYS != errno)
		return rc;
	tprintf("sendmmsg(%d, %p, %u, MSG_DONTROUTE|MSG_NOSIGNAL)"
		" = -1 ENOSYS (%m)\n", fd, vec, vlen);
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
#ifdef __NR_recvmmsg
	rc = syscall(__NR_recvmmsg, (long) fd, vec, (unsigned long) vlen,
		     (unsigned long) flags, timeout);
	if (rc >= 0 || ENOSYS != errno)
		return rc;
	tprintf("recvmmsg(%d, %p, %u, MSG_DONTWAIT, NULL)"
		" = -1 ENOSYS (%m)\n", fd, vec, vlen);
#endif
#ifdef HAVE_RECVMMSG
	rc = recvmmsg(fd, vec, vlen, flags, timeout);
#endif
	return rc;
}

int
main(void)
{
	tprintf("%s", "");

	const int R = 0, W = 1;
	int sv[2];
	if (socketpair(AF_UNIX, SOCK_DGRAM, 0, sv))
		perror_msg_and_skip("socketpair");
	assert(R == sv[0]);
	assert(W == sv[1]);

	static const char one[] = "one";
	static const char two[] = "two";
	static const char three[] = "three";
	static const char ascii_one[] = "6f 6e 65";
	static const char ascii_two[] = "74 77 6f";
	static const char ascii_three[] = "74 68 72 65 65";

	void *copy_one = tail_memdup(one, LENGTH_OF(one));
	void *copy_two = tail_memdup(two, LENGTH_OF(two));
	void *copy_three = tail_memdup(three, LENGTH_OF(three));

	struct iovec iov[] = {
		{
			.iov_base = copy_one,
			.iov_len = LENGTH_OF(one)
		}, {
			.iov_base = copy_two,
			.iov_len = LENGTH_OF(two)
		}, {
			.iov_base = copy_three,
			.iov_len = LENGTH_OF(three)
		}
	};
	struct iovec *copy_iov = tail_memdup(iov, sizeof(iov));

	struct mmsghdr mmh[] = {
		{
			.msg_hdr = {
				.msg_iov = copy_iov + 0,
				.msg_iovlen = 2,
			}
		}, {
			.msg_hdr = {
				.msg_iov = copy_iov + 2,
				.msg_iovlen = 1,
			}
		}
	};
	void *copy_mmh = tail_memdup(mmh, sizeof(mmh));
# define n_mmh ((unsigned int) (sizeof(mmh)/sizeof(mmh[0])))

	int r = send_mmsg(W, copy_mmh, n_mmh, MSG_DONTROUTE | MSG_NOSIGNAL);
	if (r < 0 && errno == ENOSYS)
		perror_msg_and_skip("sendmmsg");
	assert(r == (int) n_mmh);
	assert(close(W) == 0);
	tprintf("sendmmsg(%d, {{{msg_name(0)=NULL, msg_iov(%u)=[{\"%s\", %u}"
		", {\"%s\", %u}], msg_controllen=0, msg_flags=0}, %u}"
		", {{msg_name(0)=NULL, msg_iov(%u)=[{\"%s\", %u}]"
		", msg_controllen=0, msg_flags=0}, %u}}, %u"
		", MSG_DONTROUTE|MSG_NOSIGNAL) = %d\n"
		" = %u buffers in vector 0\n"
		" * %u bytes in buffer 0\n"
		" | 00000  %-48s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000  %-48s  %-16s |\n"
		" = %u buffers in vector 1\n"
		" * %u bytes in buffer 0\n"
		" | 00000  %-48s  %-16s |\n",
		W, 2, one, LENGTH_OF(one), two, LENGTH_OF(two),
		LENGTH_OF(one) + LENGTH_OF(two),
		1, three, LENGTH_OF(three), LENGTH_OF(three),
		n_mmh, r,
		2, LENGTH_OF(one), ascii_one, one,
		LENGTH_OF(two), ascii_two, two,
		1, LENGTH_OF(three), ascii_three, three);

	assert(recv_mmsg(R, copy_mmh, n_mmh, MSG_DONTWAIT, NULL) == (int) n_mmh);
	assert(close(R) == 0);
	tprintf("recvmmsg(%d, {{{msg_name(0)=NULL, msg_iov(%u)=[{\"%s\", %u}"
		", {\"%s\", %u}], msg_controllen=0, msg_flags=0}, %u}"
		", {{msg_name(0)=NULL, msg_iov(%u)=[{\"%s\", %u}]"
		", msg_controllen=0, msg_flags=0}, %u}}, %u"
		", MSG_DONTWAIT, NULL) = %d (left NULL)\n"
		" = %u buffers in vector 0\n"
		" * %u bytes in buffer 0\n"
		" | 00000  %-48s  %-16s |\n"
		" * %u bytes in buffer 1\n"
		" | 00000  %-48s  %-16s |\n"
		" = %u buffers in vector 1\n"
		" * %u bytes in buffer 0\n"
		" | 00000  %-48s  %-16s |\n",
		R, 2, one, LENGTH_OF(one), two, LENGTH_OF(two),
		LENGTH_OF(one) + LENGTH_OF(two),
		1, three, LENGTH_OF(three), LENGTH_OF(three),
		n_mmh, r,
		2, LENGTH_OF(one), ascii_one, one,
		LENGTH_OF(two), ascii_two, two,
		1, LENGTH_OF(three), ascii_three, three);

	tprintf("+++ exited with 0 +++\n");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("(__NR_sendmmsg || HAVE_SENDMMSG) && (__NR_recvmmsg || HAVE_RECVMMSG)")

#endif
