/*
 * Copyright (c) 2016 Anchit Jain <anchitjain1234@gmail.com>
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_chmod

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
	create_and_enter_subdir("chmod_subdir");

	char *my_secontext = SECONTEXT_PID_MY();

	static const char sample[] = "chmod_test_file";
	(void) unlink(sample);
	if (open(sample, O_CREAT|O_RDONLY, 0400) < 0)
		perror_msg_and_fail("open: %s", sample);

	long rc = syscall(__NR_chmod, sample, 0600);
	printf("%s%s(\"%s\"%s, 0600) = %s\n",
	       my_secontext, "chmod",
	       sample, SECONTEXT_FILE(sample),
	       sprintrc(rc));

	if (unlink(sample))
		perror_msg_and_fail("unlink: %s", sample);

	rc = syscall(__NR_chmod, sample, 051);
	printf("%s%s(\"%s\", 051) = %s\n",
	       my_secontext, "chmod",
	       sample,
	       sprintrc(rc));

	rc = syscall(__NR_chmod, sample, 004);
	printf("%s%s(\"%s\", 004) = %s\n",
	       my_secontext, "chmod",
	       sample,
	       sprintrc(rc));

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_chmod")

#endif
