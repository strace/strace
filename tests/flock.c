/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_flock

# include <stdio.h>
# include <sys/file.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long fd = (long int) 0xdeadbeefffffffffULL;

	long rc = syscall(__NR_flock, fd, LOCK_SH);
	printf("flock(%d, LOCK_SH) = %ld %s (%m)\n",
	       (int) fd, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_flock")

#endif
