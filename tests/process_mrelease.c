/*
 * Check decoding of process_mrelease syscall.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef FD0_STR
# define FD0_STR ""
#endif

static const char *errstr;

static long
sys_process_mrelease(int pidfd, unsigned int flags)
{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	kernel_ulong_t arg1 = fill | (unsigned int) pidfd;
	kernel_ulong_t arg2 = fill | flags;
	kernel_ulong_t arg3 = fill | 0xdeedefed;
	kernel_ulong_t arg4 = fill | 0xdebeefed;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_process_mrelease,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	sys_process_mrelease(-1, 0);
	printf("process_mrelease(-1, 0) = %s\n", errstr);

	sys_process_mrelease(0, 0xfacefeed);
	printf("process_mrelease(0" FD0_STR ", 0xfacefeed) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
