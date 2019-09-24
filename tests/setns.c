/*
 * Check decoding of setns syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#ifdef __NR_setns

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t bogus_fd =
		(kernel_ulong_t) 0xfeedfacedeadc0deULL;

	static struct {
		kernel_ulong_t val;
		const char *str;
	} nstypes[] = {
		{ (kernel_ulong_t) 0xdefaced100000000ULL, "0" },
		{ (kernel_ulong_t) 0xbadc0dedfeedfaceULL,
			"0xfeedface /* CLONE_NEW??? */" },
		{ (kernel_ulong_t) 0xca75f15702000000ULL, "CLONE_NEWCGROUP" },
	};

	long rc;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(nstypes); i++) {
		rc = syscall(__NR_setns, bogus_fd, nstypes[i].val);
		printf("setns(%d, %s) = %s\n",
			(int) bogus_fd, nstypes[i].str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_setns");

#endif
