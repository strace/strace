/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_mlock && defined __NR_munlock

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const int size = 1024;
	const char *addr = tail_alloc(size);

	long rc = syscall(__NR_mlock, addr, size);
	printf("mlock(%p, %d) = %s\n", addr, size, sprintrc(rc));

	rc = syscall(__NR_munlock, addr, size);
	printf("munlock(%p, %d) = %s\n", addr, size, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mlock && __NR_munlock")

#endif
