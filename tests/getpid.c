/*
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#if defined __NR_getpid && (!defined __NR_getxpid || __NR_getxpid != __NR_getpid)

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	PIDNS_TEST_INIT;

	pidns_print_leader();
	printf("getpid() = %d%s\n", (int) syscall(__NR_getpid),
		pidns_pid2str(PT_TGID));
	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpid")

#endif
