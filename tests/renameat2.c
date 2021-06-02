/*
 * Check decoding of renameat2 syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
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
	static const char oldpath[] = "renameat2_oldpath";
	static const char newpath[] = "renameat2_newpath";
	const unsigned long olddirfd =
		(unsigned long) 0xfacefeedffffffff;
	const unsigned long newdirfd =
		(unsigned long) 0xfacefeed00000000 | -100U;

	long rc = syscall(__NR_renameat2,
			  olddirfd, oldpath, newdirfd, newpath, 1);
	printf("renameat2(%d, \"%s\", AT_FDCWD, \"%s\", RENAME_NOREPLACE)"
	       " = %ld %s (%m)\n",
	       (int) olddirfd, oldpath, newpath, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
