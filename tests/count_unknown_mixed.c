/*
 * Check that -c summary includes both known and unknown syscalls.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "nsyscalls.h"

int
main(void)
{
	const unsigned long scno = ARRAY_SIZE(syscallent);

	(void) invoke_syscall(scno);
	(void) syscall(__NR_getpid);

	printf("syscall_%#lx\n", scno | SYSCALL_BIT);
	printf("getpid\n");

	return 0;
}
