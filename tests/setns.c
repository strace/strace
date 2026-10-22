/*
 * Check decoding of setns syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

#include <linux/sched.h>

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
		{ ARG_STR(0) },
		{ ARG_STR(CLONE_NEWTIME) },
		{ ARG_STR(CLONE_NEWNS) },
		{ ARG_STR(CLONE_NEWCGROUP) },
		{ ARG_STR(CLONE_NEWUTS) },
		{ ARG_STR(CLONE_NEWIPC) },
		{ ARG_STR(CLONE_NEWUSER) },
		{ ARG_STR(CLONE_NEWPID) },
		{ ARG_STR(CLONE_NEWNET) },
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
