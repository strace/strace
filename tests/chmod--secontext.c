/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

/*
 * This test is designed to be executed with the following strace options:
 * --secontext[=full]
 */

#if defined __NR_chmod && defined HAVE_SELINUX_RUNTIME

# include <fcntl.h>
# include <sys/stat.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# include "selinux.c"

int
main(void)
{
	static const char fname[] = "chmod_test_file";

	unlink(fname);
	if (open(fname, O_CREAT|O_RDONLY, 0400) == -1)
		perror_msg_and_fail("open");

	long rc = syscall(__NR_chmod, fname, 0600);
	printf("%schmod(\"%s\"%s, 0600) = %s\n",
	       SELINUX_MYCONTEXT(),
	       fname, SELINUX_FILECONTEXT(fname),
	       sprintrc(rc));

	if (unlink(fname))
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_chmod, fname, 051);
	printf("%schmod(\"%s\", 051) = %s\n",
	       SELINUX_MYCONTEXT(),
	       fname,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_chmod && HAVE_SELINUX_RUNTIME")

#endif
