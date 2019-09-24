/*
 * This file is part of xetpgid strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_getpgid && defined __NR_setpgid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const int pid = getpid();
	long rc = syscall(__NR_getpgid, F8ILL_KULONG_MASK | pid);
	printf("getpgid(%d) = %ld\n", pid, rc);

	rc = syscall(__NR_setpgid, F8ILL_KULONG_MASK, F8ILL_KULONG_MASK | pid);
	printf("setpgid(0, %d) = %ld\n", pid, rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpgid && __NR_setpgid")

#endif
