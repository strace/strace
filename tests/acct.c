/*
 * Check decoding of acct syscall.
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
	const char sample[] = "acct_sample";

	long rc = syscall(__NR_acct, sample);
	printf("acct(\"%s\") = %s\n", sample, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
