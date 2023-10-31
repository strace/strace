/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR__llseek

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long high = 0xfacefeed;
	const unsigned long low = 0xdeadbeef;
	const long long offset = 0xfacefeeddeadbeefLL;
	unsigned long long result;

	long rc = syscall(__NR__llseek, -1, high, low, &result, SEEK_SET);
	printf("_llseek(-1, %lld, %p, SEEK_SET) = %s\n",
	       offset, &result, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR__llseek")

#endif
