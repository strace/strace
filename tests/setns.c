/*
 * Check decoding of setns syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static long
k_setns(const int fd, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0x0defaced00000000ULL;
	const kernel_ulong_t bad  = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | (unsigned int) fd;
	const kernel_ulong_t arg2 = fill | flags;
	const long rc = syscall(__NR_setns,
				arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	static const int bogus_fd = 0xdeadc0de;

	static const struct strval32 nstypes[] = {
		{ 0, "0" },
		{ 0x00000080U, "CLONE_NEWTIME" },
		{ 0x00020000U, "CLONE_NEWNS" },
		{ 0x02000000U, "CLONE_NEWCGROUP" },
		{ 0x04000000U, "CLONE_NEWUTS" },
		{ 0x08000000U, "CLONE_NEWIPC" },
		{ 0x10000000U, "CLONE_NEWUSER" },
		{ 0x20000000U, "CLONE_NEWPID" },
		{ 0x40000000U, "CLONE_NEWNET" },
		{ 0x81fdff7fU, "0x81fdff7f /* CLONE_NEW??? */" },
		{ -1U,
		  "CLONE_NEWTIME|CLONE_NEWNS|CLONE_NEWCGROUP|CLONE_NEWUTS|"
		  "CLONE_NEWIPC|CLONE_NEWUSER|CLONE_NEWPID|CLONE_NEWNET|"
		  "0x81fdff7f" },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(nstypes); ++i) {
		k_setns(bogus_fd, nstypes[i].val);
		printf("setns(%d, %s) = %s\n",
		       bogus_fd, nstypes[i].str, errstr);
	}

	puts("+++ exited with 0 +++");

	return 0;
}
