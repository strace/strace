/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_link

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const char sample_1[] = "link_sample_old";
	static const char sample_2[] = "link_sample_new";

	long rc = syscall(__NR_link, sample_1, sample_2);
	printf("link(\"%s\", \"%s\") = %ld %s (%m)\n",
	       sample_1, sample_2, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_link")

#endif
