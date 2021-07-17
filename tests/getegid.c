/*
 * Check decoding of getegid syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	printf("getegid() = %s\n", sprintrc(syscall(__NR_getegid)));
	return 0;
}
