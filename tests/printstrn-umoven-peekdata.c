/*
 * Test PTRACE_PEEKDATA-based printstrn/umoven.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
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

	test_printstrn(DEFAULT_STRLEN);

	puts("+++ exited with 0 +++");
	return 0;
}
