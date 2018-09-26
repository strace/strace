/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_syslog

# include <stdio.h>
# include <unistd.h>

# define SYSLOG_ACTION_READ 2
# define SYSLOG_ACTION_SIZE_BUFFER 10

int
main(void)
{
	const long addr = (long) 0xfacefeeddeadbeefULL;

	int rc = syscall(__NR_syslog, SYSLOG_ACTION_READ, addr, -1);
	printf("syslog(2 /* SYSLOG_ACTION_READ */, %#lx, -1) = %s\n",
	       addr, sprintrc(rc));

	rc = syscall(__NR_syslog, SYSLOG_ACTION_SIZE_BUFFER, NULL, 10);
	printf("syslog(10 /* SYSLOG_ACTION_SIZE_BUFFER */, NULL, 10) = %s\n",
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_syslog")

#endif
