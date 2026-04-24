/*
 * Check decoding of rseq_slice_yield syscall.
 *
 * Copyright (c) 2026 The strace developers.
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
	printf("rseq_slice_yield() = %s\n",
	       sprintrc(syscall(__NR_rseq_slice_yield)));

	puts("+++ exited with 0 +++");
	return 0;
}
