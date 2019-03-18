/*
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

void
fill_memory_ex(void *ptr, size_t size, unsigned char start,
	       unsigned int period)
{
	unsigned char *p = ptr;
	size_t i;

	for (i = 0; i < size; i++) {
		p[i] = start + i % period;
	}
}

void
fill_memory(void *ptr, size_t size)
{
	fill_memory_ex(ptr, size, 0x80, 0x80);
}
