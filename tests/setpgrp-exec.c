/*
 * Copyright (c) 2020 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>

int main(int argc, char **argv)
{
	if (argc < 2)
		error_msg_and_fail("argc < 2");

	if (setpgid(0, 0))
		perror_msg_and_fail("setpgid");

	(void) execvp(argv[1], argv + 1);

	perror_msg_and_fail("execvp: %s", argv[1]);
}
