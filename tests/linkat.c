/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_linkat

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample_1[] = "linkat_sample_old";
	static const char sample_2[] = "linkat_sample_new";
	const long fd_old = (long) 0xdeadbeefffffffffULL;
	const long fd_new = (long) 0xdeadbeeffffffffeULL;

	long rc = syscall(__NR_linkat, fd_old, sample_1, fd_new, sample_2, 0);
	printf("linkat(%d, \"%s\", %d, \"%s\", 0) = %ld %s (%m)\n",
	       (int) fd_old, sample_1, (int) fd_new, sample_2,
	       rc, errno2name());

	rc = syscall(__NR_linkat, -100, sample_1, -100, sample_2, -1L);
	printf("linkat(%s, \"%s\", %s, \"%s\", %s) = %ld %s (%m)\n",
	       "AT_FDCWD", sample_1, "AT_FDCWD", sample_2,
	       "AT_SYMLINK_NOFOLLOW|AT_REMOVEDIR|AT_SYMLINK_FOLLOW"
	       "|AT_NO_AUTOMOUNT|AT_EMPTY_PATH|AT_RECURSIVE|0xffff60ff",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_linkat")

#endif
