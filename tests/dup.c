/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	int rc = dup(-1);
	printf("dup(-1) = %d %s (%m)\n", rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
