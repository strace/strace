/*
 * Check decoding of setdomainname syscall.
 *
 * Copyright (c) 2016-2023 The strace developers.
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
	long rc = syscall(__NR_setdomainname, 0, 63);
	printf("setdomainname(NULL, 63) = %s\n", sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
