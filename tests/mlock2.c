/*
 * Check decoding of mlock2 syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
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
	const unsigned long addr = (unsigned long) 0xfacefeeddeadbeefULL;
	const unsigned long len = (unsigned long) 0xcafef00dbadc0dedULL;

	long rc = syscall(__NR_mlock2, addr, len, -1UL);
	printf("mlock2(%#lx, %lu, MLOCK_ONFAULT|0xfffffffe)"
	       " = %s\n", addr, len, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
