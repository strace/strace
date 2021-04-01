/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_access

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("access_subdir");

	static const char sample[] = "access_sample";

	long rc = syscall(__NR_access, sample, F_OK);
	printf("access(\"%s\", F_OK) = %ld %s (%m)\n",
	       sample, rc, errno2name());

	rc = syscall(__NR_access, sample, R_OK|W_OK|X_OK);
	printf("access(\"%s\", R_OK|W_OK|X_OK) = %ld %s (%m)\n",
	       sample, rc, errno2name());

	leave_and_remove_subdir();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_access")

#endif
