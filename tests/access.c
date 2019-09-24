/*
 * Copyright (c) 2016-2019 The strace developers.
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
	static const char sample[] = "access_sample";

	long rc = syscall(__NR_access, sample, F_OK);
	printf("access(\"%s\", F_OK) = %ld %s (%m)\n",
	       sample, rc, errno2name());

	rc = syscall(__NR_access, sample, R_OK|W_OK|X_OK);
	printf("access(\"%s\", R_OK|W_OK|X_OK) = %ld %s (%m)\n",
	       sample, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_access")

#endif
