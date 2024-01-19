/*
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

/* Check decoding of shutdown syscall. */

#include "tests.h"
#include <stdio.h>
#include <sys/socket.h>

int
main(void)
{
	int rc = shutdown(-1, SHUT_RDWR);
	printf("shutdown(-1, SHUT_RDWR) = %s\n", sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
