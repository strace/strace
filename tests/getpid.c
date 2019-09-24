/*
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_getpid && (!defined __NR_getxpid || __NR_getxpid != __NR_getpid)

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getpid() = %ld\n", syscall(__NR_getpid));
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpid")

#endif
