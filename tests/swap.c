/*
 * Check decoding of swapon and swapoff tests.
 *
 * Copyright (c) 2016-2021 The strace developers.
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
	printf("swapon(\"%s\", %s) = %ld %s (%m)\n",
	       sample, "0", rc, errno2name());

	rc = syscall(__NR_swapon, sample, 42);
	printf("swapon(\"%s\", %s) = %ld %s (%m)\n",
	       sample, "42", rc, errno2name());

	rc = syscall(__NR_swapon, sample, SWAP_FLAG_PREFER);
	printf("swapon(\"%s\", %s) = %ld %s (%m)\n",
	       sample, "SWAP_FLAG_PREFER|0", rc, errno2name());

	rc = syscall(__NR_swapon, sample, SWAP_FLAG_PREFER | 42);
	printf("swapon(\"%s\", %s) = %ld %s (%m)\n",
	       sample, "SWAP_FLAG_PREFER|42", rc, errno2name());

	rc = syscall(__NR_swapon, sample, -1L);
	printf("swapon(\"%s\", %s) = %ld %s (%m)\n",
	       sample,
	       "SWAP_FLAG_PREFER|SWAP_FLAG_DISCARD|SWAP_FLAG_DISCARD_ONCE"
	       "|SWAP_FLAG_DISCARD_PAGES|0xfff80000|32767",
	       rc, errno2name());

	rc = syscall(__NR_swapoff, sample);
	printf("swapoff(\"%s\") = %ld %s (%m)\n",
	       sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
