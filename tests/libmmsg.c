/*
 * Wrappers for recvmmsg and sendmmsg syscalls.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include "scno.h"

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
