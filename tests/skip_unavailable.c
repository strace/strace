/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <sys/stat.h>
#include <unistd.h>

void
skip_if_unavailable(const char *const path)
{
	struct stat st;

	if (stat(path, &st))
		perror_msg_and_skip("stat: %s", path);
}
