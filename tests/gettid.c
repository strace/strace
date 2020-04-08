/*
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include "scno.h"

int
main(void)
{
	printf("gettid() = %ld\n", syscall(__NR_gettid));
	puts("+++ exited with 0 +++");
	return 0;
}
