/*
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#ifdef __NR_getpgrp

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	PIDNS_TEST_INIT;

	pidns_print_leader();
	printf("getpgrp() = %d%s\n", (int) syscall(__NR_getpgrp),
		pidns_pid2str(PT_PGID));

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getpgrp")

#endif
