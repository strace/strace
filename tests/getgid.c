/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_getgid && (!defined __NR_getxgid || __NR_getxgid != __NR_getgid)

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getgid() = %ld\n", syscall(__NR_getgid));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getgid")

#endif
