/*
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_getxpid

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	long id = syscall(__NR_getxpid);
	pid_t ppid = getppid();

	printf("getxpid() = %ld (ppid %ld)\n", id, (long) ppid);
	printf("getxpid() = %ld (ppid %ld)\n", id, (long) ppid);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getxpid")

#endif
