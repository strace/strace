/*
 * Check status=failed filtering for failed and successful syscalls.
 *
 * Copyright (c) 2019 Intel Corporation
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
	static const char sample_valid[] = ".";
	static const char sample_invalid[] = "";

	test_status_chdir(sample_valid, 0, 1);
	test_status_chdir(sample_invalid, 0, 1);
	puts("+++ exited with 0 +++");
	return 0;
}
