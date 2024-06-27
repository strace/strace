/*
 * Check decoding of mseal syscall.
 *
 * Copyright (c) 2015-2024 Dmitry V. Levin <ldv@strace.io>
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
	const unsigned long flags = -1UL;

	long rc = syscall(__NR_mseal, addr, len, flags);
	printf("mseal(%#lx, %lu, %#lx) = %s\n",
	       addr, len, flags, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
