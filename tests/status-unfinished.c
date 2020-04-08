/*
 * Check basic -e status=unfinished syscall filtering.
 *
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>

int
main(void)
{
	puts("exit_group(0) = ?\n"
	     "+++ exited with 0 +++");
	return 0;
}
