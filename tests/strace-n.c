/*
 * Test strace's -n option.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define SC_listen 4

int
main(void)
{
	int rc;

#if defined __NR_socketcall
	if (syscall(__NR_socketcall, 0L, 0L, 0L, 0L, 0L) < 0
		&& EINVAL == errno)
	{
		const long args[] = { 0, 0 };
		rc = syscall(__NR_socketcall, SC_listen, args);
		printf("[%4u] listen(0, 0) = %s\n", __NR_socketcall, sprintrc(rc));
	}
#endif

#if defined __NR_listen
	rc = syscall(__NR_listen, 0, 0);
	printf("[%4u] listen(0, 0) = %s\n", __NR_listen, sprintrc(rc));
#endif

	return 0;
}
