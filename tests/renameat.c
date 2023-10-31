/*
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_renameat

# include <stdio.h>
# include <unistd.h>

# define OLD_FILE "renameat_old"
# define NEW_FILE "renameat_new"

int
main(void)
{
	const long int fd_old = (long int) 0xdeadbeefffffffffULL;
	const long int fd_new = (long int) 0xdeadbeeffffffffeULL;

	long rc = syscall(__NR_renameat, fd_old, OLD_FILE, fd_new, NEW_FILE);
	printf("renameat(%d, \"%s\", %d, \"%s\") = %s\n",
	       (int) fd_old, OLD_FILE, (int) fd_new, NEW_FILE,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_renameat")

#endif
