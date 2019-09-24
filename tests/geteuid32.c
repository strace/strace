/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_geteuid32

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	printf("geteuid32() = %ld\n", syscall(__NR_geteuid32));
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_geteuid32")

#endif
