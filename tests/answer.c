/*
 * Check decoding of fault injected exit_group syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>
#include "scno.h"

int
main(void)
{
	static const kernel_ulong_t answer =
		(kernel_ulong_t) 0xbadc0ded0000002aULL;

	syscall(__NR_exit_group, answer);
	syscall(__NR_exit, answer);

	return 1;
}
