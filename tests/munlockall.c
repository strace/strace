/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <sys/mman.h>

int
main(void)
{
	printf("munlockall() = %s\n", sprintrc(munlockall()));

	puts("+++ exited with 0 +++");
	return 0;
}
