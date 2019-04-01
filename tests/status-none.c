/*
 * Check basic -e status=none syscall filtering.
 *
 * Copyright (c) 2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>

int
main(void)
{
	puts("+++ exited with 0 +++");
	return 0;
}
