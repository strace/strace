/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_symlinkat

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd = (long int) 0xdeadbeefffffffffULL;
	static const char oldpath[] = "symlink_old";
	static const char newpath[] = "symlink_new";

	long rc = syscall(__NR_symlinkat, oldpath, fd, newpath);
	printf("symlinkat(\"%s\", %d, \"%s\") = %ld %s (%m)\n",
	       oldpath, (int) fd, newpath, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_symlinkat")

#endif
