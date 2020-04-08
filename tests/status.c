/*
 * Helper function to check -e status option.
 *
 * Copyright (c) 2019 Intel Corporation
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>

void
test_status_chdir(const char *dir, bool print_success, bool print_fail)
{
	long rc = chdir(dir);
	if ((rc == -1 && print_fail) || (rc != -1 && print_success))
		printf("chdir(\"%s\") = %s\n", dir, sprintrc(rc));
}
