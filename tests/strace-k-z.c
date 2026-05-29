/*
 * Check that stack tracing combined with syscall status filtering does not
 * abort strace with "bug: unprinted entries in queue".
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>

int
main(void)
{
	char *const argv[] = { (char *) "does_not_exist", NULL };
	char *const envp[] = { NULL };

	execve("does_not_exist", argv, envp);
	_exit(0);
}
