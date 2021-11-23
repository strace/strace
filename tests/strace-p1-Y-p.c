/*
 * This file is part of strace-p-Y-p strace test.
 *
 * Copyright (c) 2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef MY_COMM
# define MY_COMM "strace-p1-Y-p"
#endif

int
main(int ac, char **av)
{
	if (ac < 2)
		error_msg_and_fail("missing operand");

	if (ac > 2)
		error_msg_and_fail("extra operand");

	static const char proc_self_comm[] = "/proc/self/comm";
	skip_if_unavailable(proc_self_comm);

	int seconds = atoi(av[1]);
	FILE *fp = fdopen(3, "a");
	if (!fp)
		error_msg_and_fail("fdopen(3, \"a\")");

	fprintf(fp, "%u<%s> +++ exited with 0 +++\n", getpid(), MY_COMM);
	fclose(fp);

	if (sleep(seconds))
		perror_msg_and_skip("sleep: %d", seconds);

	return 0;
}
