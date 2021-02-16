/*
 * Test regular printpath/umovestr.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "test_ucopy.h"
#include <limits.h>
#include <stdio.h>

int
main(void)
{
	if (!test_process_vm_readv())
		perror_msg_and_skip("process_vm_readv");

	test_printpath(PATH_MAX);

	puts("+++ exited with 0 +++");
	return 0;
}
