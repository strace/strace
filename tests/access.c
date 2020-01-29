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

# include "secontext.h"

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("access_subdir");

	char *my_secontext = SECONTEXT_PID_MY();

	static const char sample[] = "access_sample";
	(void) unlink(sample);
	if (open(sample, O_CREAT|O_RDONLY, 0400) == -1)
		perror_msg_and_fail("open: %s", sample);

	long rc = syscall(__NR_access, sample, F_OK);
	printf("%s%s(\"%s\"%s, F_OK) = %s\n",
	       my_secontext, "access",
	       sample, SECONTEXT_FILE(sample),
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink: %s", sample);

	rc = syscall(__NR_access, sample, R_OK|W_OK|X_OK);
	printf("%s%s(\"%s\", R_OK|W_OK|X_OK) = %s\n",
	       my_secontext, "access",
	       sample,
	       sprintrc(rc));

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_access")

#endif
