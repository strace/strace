/*
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include "scno.h"
#include "pidns.h"

int
main(void)
{
	PIDNS_TEST_INIT;

	pidns_print_leader();
	printf("gettid() = %d%s\n", (int) syscall(__NR_gettid),
		pidns_pid2str(PT_TID));
	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
