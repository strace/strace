/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_getegid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("getegid() = %ld\n", syscall(__NR_getegid));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getegid")

#endif
