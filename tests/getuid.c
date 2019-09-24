/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_getuid && (!defined __NR_getxuid || __NR_getxuid != __NR_getuid)

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getuid() = %ld\n", syscall(__NR_getuid));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getuid")

#endif
