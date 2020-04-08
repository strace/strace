/*
 * Invoke a socket syscall, either directly or via __NR_socketcall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <unistd.h>
#include "scno.h"

/*
 * Invoke a socket syscall, either directly or via __NR_socketcall.
 * if nr == -1, no direct syscall invocation will be made.
 */
int
socketcall(const int nr, const int call,
	   long a1, long a2, long a3, long a4, long a5)
{
	int rc = -1;
	errno = ENOSYS;

#ifdef __NR_socketcall
	static int have_socketcall = -1;

	if (have_socketcall < 0) {
		if (syscall(__NR_socketcall, 0L, 0L, 0L, 0L, 0L) < 0
		    && EINVAL == errno) {
			have_socketcall = 1;
		} else {
			have_socketcall = 0;
		}
	}

	if (have_socketcall) {
		const long args[] = { a1, a2, a3, a4, a5 };
		rc = syscall(__NR_socketcall, call, args);
	} else
#endif
	{
		if (nr != -1)
			rc = syscall(nr, a1, a2, a3, a4, a5);
	}

	return rc;
}
