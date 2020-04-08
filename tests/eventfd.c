/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <fcntl.h>
#include <unistd.h>
#include "scno.h"

#if defined __NR_eventfd2 && defined O_CLOEXEC

int
main(void)
{
	(void) close(0);
	if (syscall(__NR_eventfd2, -1L, 1 | O_CLOEXEC | O_NONBLOCK))
		perror_msg_and_skip("eventfd2");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_eventfd2 && O_CLOEXEC")

#endif
