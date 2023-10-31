/*
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_rename

# include <stdio.h>
# include <unistd.h>

# define OLD_FILE "rename_old"
# define NEW_FILE "rename_new"

int
main(void)
{
	long rc = syscall(__NR_rename, OLD_FILE, NEW_FILE);
	printf("rename(\"%s\", \"%s\") = %s\n",
	       OLD_FILE, NEW_FILE, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_rename")

#endif
