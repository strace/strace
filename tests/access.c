/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_access

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# include "selinux.c"

int
main(void)
{
	static const char sample[] = "access_sample";
	char *my_secontext = SELINUX_MYCONTEXT();

	unlink(sample);
	if (open(sample, O_CREAT|O_RDONLY, 0400) == -1)
		perror_msg_and_fail("open");

	long rc = syscall(__NR_access, sample, F_OK);
	printf("%saccess(\"%s\"%s, F_OK) = %s\n",
	       my_secontext,
	       sample, SELINUX_FILECONTEXT(sample),
	       sprintrc(rc));

	if (unlink(sample) == -1)
		perror_msg_and_fail("unlink");

	rc = syscall(__NR_access, sample, R_OK|W_OK|X_OK);
	printf("%saccess(\"%s\", R_OK|W_OK|X_OK) = %s\n",
	       my_secontext,
	       sample,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_access")

#endif
