/*
 * This file is part of clock_adjtime* strace tests.
 *
 * Copyright (c) 2016-2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

# include <stdio.h>
# include <time.h>
# include <unistd.h>

int
main(void)
{
	long rc = syscall(SYSCALL_NR, CLOCK_MONOTONIC, NULL);
	printf("%s(CLOCK_MONOTONIC, NULL) = %s\n",
	       SYSCALL_NAME, sprintrc(rc));

	void *efault = tail_alloc(1);

	rc = syscall(SYSCALL_NR, CLOCK_REALTIME, efault);
	printf("%s(CLOCK_REALTIME, %p) = %s\n",
	       SYSCALL_NAME, efault, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
