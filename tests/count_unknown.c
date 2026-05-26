/*
 * Check that -c accounts for out-of-range syscalls in the summary.
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
	const unsigned long scno1 = ARRAY_SIZE(syscallent);
	const unsigned long scno2 = scno1 + 1;

	(void) invoke_syscall(scno1);
	(void) invoke_syscall(scno2);
	(void) invoke_syscall(scno1);

	printf("syscall_%#lx\n", scno1 | SYSCALL_BIT);
	printf("syscall_%#lx\n", scno2 | SYSCALL_BIT);

	(void) syscallent;	/* workaround for clang bug #33068 */

	return 0;
}
