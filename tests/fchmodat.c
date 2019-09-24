/*
 * Check decoding of fchmodat syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fchmodat

# include <fcntl.h>
# include <sys/stat.h>
# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample[] = "fchmodat_sample";

	if (open(sample, O_RDONLY | O_CREAT, 0400) < 0)
		perror_msg_and_fail("open");

	long rc = syscall(__NR_fchmodat, -100, sample, 0600);
	printf("fchmodat(AT_FDCWD, \"%s\", 0600) = %s\n",
	       sample, sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_fchmodat, -100, sample, 051);
	printf("fchmodat(AT_FDCWD, \"%s\", 051) = %s\n",
	       sample, sprintrc(rc));

	rc = syscall(__NR_fchmodat, -100, sample, 004);
	printf("fchmodat(AT_FDCWD, \"%s\", 004) = %s\n",
	       sample, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fchmodat")

#endif
