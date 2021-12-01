/*
 * Copyright (c) 2016-2021 The strace developers.
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

	for (typeof(size) i = 0; i < size; ++i) {
		p[i] = start + i % period;
	}
}

void
fill_memory(void *ptr, size_t size)
{
	fill_memory_ex(ptr, size, 0x80, 0x80);
}

void
fill_memory16_ex(void *ptr, size_t size, uint16_t start,
	         unsigned int period)
{
	uint16_t *p = ptr;

	for (typeof(size) i = 0; i < size / sizeof(uint16_t); ++i) {
		p[i] = start + i % period;
	}
}

void
fill_memory16(void *ptr, size_t size)
{
	fill_memory16_ex(ptr, size, 0x80c0, 0x8000);
}

void
fill_memory32_ex(void *ptr, size_t size, uint32_t start,
	         unsigned int period)
{
	uint32_t *p = ptr;

	for (typeof(size) i = 0; i < size / sizeof(uint32_t); ++i) {
		p[i] = start + i % period;
	}
}

void
fill_memory32(void *ptr, size_t size)
{
	fill_memory32_ex(ptr, size, 0x80a0c0e0, 0x80000000);
}

void
fill_memory64_ex(void *ptr, size_t size, uint64_t start, uint64_t period)
{
	uint64_t *p = ptr;

	for (typeof(size) i = 0; i < size / sizeof(uint64_t); i++) {
		p[i] = start + i % period;
	}
}

void
fill_memory64(void *ptr, size_t size)
{
	fill_memory64_ex(ptr, size,
			 0x8090a0b0c0d0e0f0ULL, 0x8000000000000000ULL);
}
