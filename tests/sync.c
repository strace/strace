/*
 * Check decoding of sync syscall.
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
	printf("sync() = %ld\n", syscall(__NR_sync));

	puts("+++ exited with 0 +++");
	return 0;
}
