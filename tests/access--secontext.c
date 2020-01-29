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

#if defined __NR_access && defined HAVE_SELINUX_RUNTIME

# include <fcntl.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>

# include "selinux.c"

int
main(void)
{
	static const char sample[] = "access_sample";

	unlink(sample);
	if (open(sample, O_CREAT|O_RDONLY, 0400) == -1)
		perror_msg_and_fail("open");

	long rc = syscall(__NR_access, sample, F_OK);
	printf("%saccess(\"%s\"%s, F_OK) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(rc));

	if (unlink(sample) == -1)
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_access, sample, F_OK);
	printf("%saccess(\"%s\", F_OK) = %s\n",
	       SELINUX_MYCONTEXT(),
	       sample,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_access && HAVE_SELINUX_RUNTIME")

#endif
