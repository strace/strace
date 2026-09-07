/*
 * Check decoding of fchroot syscall.
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

#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#ifndef FD0_STR
# define FD0_STR ""
#endif

#ifndef FD_FAILFS_ROOT
# define FD_FAILFS_ROOT -10004
#endif

static const char *errstr;

static long
k_fchroot(const unsigned int fd, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | fd;
	const kernel_ulong_t arg2 = fill | flags;
	const long rc = syscall(__NR_fchroot, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	k_fchroot(-1, 0);
	printf("fchroot(-1, 0) = %s\n", errstr);

	k_fchroot(FD_FAILFS_ROOT, 0);
	printf("fchroot(FD_FAILFS_ROOT, 0) = %s\n", errstr);

	k_fchroot(0, 0xfacefeed);
	printf("fchroot(0" FD0_STR ", 0xfacefeed) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
