/*
 * Check that -c accounts for many distinct out-of-range syscalls.
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
	for (unsigned i = 0; i < 17; i++) {
		const unsigned long scno = ARRAY_SIZE(syscallent) + i;

		(void) invoke_syscall(scno);
		printf("syscall_%#lx\n", scno | SYSCALL_BIT);
	}

	(void) syscallent;	/* workaround for clang bug #33068 */

	return 0;
}
