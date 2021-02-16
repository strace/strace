/*
 * Test PTRACE_PEEKDATA-based printpath/umovestr.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "test_ucopy.h"
#include <stdio.h>

int
main(int ac, char **av)
{
	if (ac < 2 && test_process_vm_readv())
		error_msg_and_skip("process_vm_readv is available");

	if (!test_ptrace_peekdata())
		perror_msg_and_skip("PTRACE_PEEKDATA");

	test_printpath(sizeof(long) * 4);

	puts("+++ exited with 0 +++");
	return 0;
}
