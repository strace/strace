/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <unistd.h>

size_t
get_page_size(void)
{
	static size_t page_size;

	if (!page_size)
		page_size = sysconf(_SC_PAGESIZE);

	return page_size;
}
