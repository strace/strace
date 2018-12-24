/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <string.h>
#include <sys/mman.h>

void *
tail_alloc(const size_t size)
{
	const size_t page_size = get_page_size();
	const size_t len = (size + page_size - 1) & -page_size;
	const size_t alloc_size = len + 6 * page_size;

	void *p = mmap(NULL, alloc_size, PROT_READ | PROT_WRITE,
		       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (MAP_FAILED == p)
		perror_msg_and_fail("mmap(%zu)", alloc_size);

	void *start_work = p + 3 * page_size;
	void *tail_guard = start_work + len;

	if (munmap(p, page_size) ||
	    munmap(p + 2 * page_size, page_size) ||
	    munmap(tail_guard, page_size) ||
	    munmap(tail_guard + 2 * page_size, page_size))
		perror_msg_and_fail("munmap");

	memset(start_work, 0xff, len);
	return tail_guard - size;
}

void *
tail_memdup(const void *p, const size_t size)
{
	void *dest = tail_alloc(size);
	memcpy(dest, p, size);
	return dest;
}
