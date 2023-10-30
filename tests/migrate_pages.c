/*
 * Check decoding of migrate_pages syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	PIDNS_TEST_INIT;

	const long pid = (long) 0xfacefeed00000000ULL | getpid();
	long rc = syscall(__NR_migrate_pages, pid, 0, 0, 0);

	pidns_print_leader();
	printf("migrate_pages(%d%s, 0, NULL, NULL) = %s\n",
		(int) pid, pidns_pid2str(PT_TGID), sprintrc(rc));

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
