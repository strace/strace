/*
 * Check decoding of eventfd2 syscall.
 *
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <unistd.h>
#include "kernel_fcntl.h"

int
main(void)
{
	(void) close(0);
	if (syscall(__NR_eventfd2, -1L, 1 | O_CLOEXEC | O_NONBLOCK))
		perror_msg_and_skip("eventfd2");
	return 0;
}
