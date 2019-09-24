/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_brk

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_brk, NULL);
	printf("brk\\(NULL\\) = %#lx\n", rc);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_brk")

#endif
