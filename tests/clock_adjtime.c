/*
 * This file is part of clock_adjtime strace test.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_clock_adjtime

# include <stdio.h>
# include <time.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(__NR_clock_adjtime, CLOCK_MONOTONIC, NULL);
	printf("clock_adjtime(CLOCK_MONOTONIC, NULL) = %ld %s (%m)\n",
	       rc, errno2name());

	void *efault = tail_alloc(1);

	rc = syscall(__NR_clock_adjtime, CLOCK_REALTIME, efault);
	printf("clock_adjtime(CLOCK_REALTIME, %p) = %ld %s (%m)\n",
	       efault, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_clock_adjtime")

#endif
