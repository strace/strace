/*
 * This file is part of alarm strace test.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_alarm

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long arg = (unsigned long) 0xffffffff0000002aULL;
	printf("alarm(%u) = %s\n", 42, sprintrc(syscall(__NR_alarm, arg)));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_alarm")

#endif
