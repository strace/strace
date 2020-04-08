/*
 * Check fflush error diagnostics.
 *
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

int
main(void)
{
	errno = ENOSPC;
	printf("%s: /dev/full: %m\n", getenv("STRACE_EXE") ?: "strace");
	return 0;
}
