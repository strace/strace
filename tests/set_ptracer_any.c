/*
 * Copyright (c) 2013-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2013-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/prctl.h>

int main(int argc, char **argv)
{
	if (argc < 2)
		return 99;
	/*
	 * Turn off restrictions on tracing if applicable.
	 * If the command is not available on this system, that's OK too.
	 */
	(void) prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY, 0, 0, 0);
	if (write(1, "\n", 1) != 1) {
		perror("write");
		return 99;
	}
	(void) execvp(argv[1], argv + 1);
	perror(argv[1]);
	return 99;
}
