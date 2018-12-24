/*
 * This file is part of alarm strace test.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_alarm

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	int rc = syscall(__NR_alarm, (unsigned long) 0xffffffff0000002aULL);
	printf("alarm(%u) = %d\n", 42, rc);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_alarm")

#endif
