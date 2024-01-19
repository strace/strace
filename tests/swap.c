/*
 * Check decoding of swapon and swapoff tests.
 *
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <sys/swap.h>
#include <unistd.h>

int
main(void)
{
	static const char sample[] = "swap.sample";
	long rc;

	rc = syscall(__NR_swapon, sample, 0);
	printf("swapon(\"%s\", %s) = %s\n",
	       sample, "0", sprintrc(rc));

	rc = syscall(__NR_swapon, sample, 42);
	printf("swapon(\"%s\", %s) = %s\n",
	       sample, "42", sprintrc(rc));

	rc = syscall(__NR_swapon, sample, SWAP_FLAG_PREFER);
	printf("swapon(\"%s\", %s) = %s\n",
	       sample, "SWAP_FLAG_PREFER|0", sprintrc(rc));

	rc = syscall(__NR_swapon, sample, SWAP_FLAG_PREFER | 42);
	printf("swapon(\"%s\", %s) = %s\n",
	       sample, "SWAP_FLAG_PREFER|42", sprintrc(rc));

	rc = syscall(__NR_swapon, sample, -1L);
	printf("swapon(\"%s\", %s) = %s\n",
	       sample,
	       "SWAP_FLAG_PREFER|SWAP_FLAG_DISCARD|SWAP_FLAG_DISCARD_ONCE"
	       "|SWAP_FLAG_DISCARD_PAGES|0xfff80000|32767",
	       sprintrc(rc));

	rc = syscall(__NR_swapoff, sample);
	printf("swapoff(\"%s\") = %s\n",
	       sample, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
