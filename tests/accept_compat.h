/*
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _STRACE_TESTS_ACCEPT_COMPAT_H_
# define _STRACE_TESTS_ACCEPT_COMPAT_H_

# include <unistd.h>
# include <sys/socket.h>
# include "scno.h"

# if defined __NR_socketcall && defined __sparc__
/*
 * Work around the fact that
 * - glibc >= 2.26 uses accept4 syscall to implement accept() call on sparc;
 * - accept syscall had not been wired up on sparc until v4.4-rc8~4^2~1.
 */
static inline int
do_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	const long args[] = { sockfd, (long) addr, (long) addrlen };

	return syscall(__NR_socketcall, 5, args);
}
# else
#  define do_accept accept
# endif

#endif /* !_STRACE_TESTS_ACCEPT_COMPAT_H_ */
