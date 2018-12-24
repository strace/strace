/*
 * A simple nanosleep based sleep(1) replacement.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdlib.h>
#include <time.h>

int
main(int ac, char **av)
{
	if (ac < 2)
		error_msg_and_fail("missing operand");

	if (ac > 2)
		error_msg_and_fail("extra operand");

	struct timespec ts = { atoi(av[1]), 0 };

	if (nanosleep(&ts, NULL))
		perror_msg_and_fail("nanosleep");

	return 0;
}
