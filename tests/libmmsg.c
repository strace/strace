/*
 * Wrappers for recvmmsg and sendmmsg syscalls.
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
#include <errno.h>
#include <asm/unistd.h>

#ifndef __NR_recvmmsg
# define __NR_recvmmsg -1
#endif
#define SC_recvmmsg 19

#ifndef __NR_sendmmsg
# define __NR_sendmmsg -1
#endif
#define SC_sendmmsg 20

int
recv_mmsg(const int fd, struct mmsghdr *const vec,
	  const unsigned int vlen, const unsigned int flags,
	  struct timespec *const timeout)
{
	int rc = socketcall(__NR_recvmmsg, SC_recvmmsg,
			    fd, (long) vec, vlen, flags, (long) timeout);

	if (rc < 0 && ENOSYS == errno)
		perror_msg_and_skip("recvmmsg");

	return rc;
}

int
send_mmsg(const int fd, struct mmsghdr *const vec,
	  const unsigned int vlen, const unsigned int flags)
{
	int rc = socketcall(__NR_sendmmsg, SC_sendmmsg,
			    fd, (long) vec, vlen, flags, 0);

	if (rc < 0 && ENOSYS == errno)
		perror_msg_and_skip("sendmmsg");

	return rc;
}
