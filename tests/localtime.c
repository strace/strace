/*
 * Check handling of localtime() returning NULL in printleader().
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "scno.h"

int
main(void)
{
	struct timespec ts;
	int pid;

	assert(!clock_gettime(CLOCK_REALTIME, &ts));

	pid = syscall(__NR_gettid);

	/* We expect localtime to fail here */
	printf("%lld.%06ld gettid() = %d\n",
	       (long long) ts.tv_sec, (long) (ts.tv_nsec / 1000), pid);

	printf("%lld.%06ld +++ exited with 0 +++\n",
	       (long long) ts.tv_sec, (long) (ts.tv_nsec / 1000));

	return 0;
}
