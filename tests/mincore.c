/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/mman.h>

static void
print_mincore(const unsigned int pages, void *const addr,
	      const size_t size, unsigned char *const vec)
{
	unsigned int i;

	if (mincore(addr, size, vec))
		perror_msg_and_skip("mincore");

	printf("mincore(%p, %zu, [", addr, size);
	for (i = 0; i < pages; ++i) {
		if (i)
			printf(", ");
		if (i >= DEFAULT_STRLEN) {
			printf("...");
			break;
		}
		printf("%u", vec[i] & 1);
	}
	puts("]) = 0");
}

static void
test_mincore(const unsigned int pages)
{
	const size_t page_size = get_page_size();
	const size_t size = pages * page_size;
	void *const addr = tail_alloc(size);
	unsigned char *const vec = tail_alloc(pages);

	mincore(addr, size, NULL);
	printf("mincore(%p, %zu, NULL) = -1 %s (%m)\n",
	       addr, size, errno2name());

	print_mincore(pages, addr, size, vec);
	if (size)
		print_mincore(pages, addr, size - page_size + 1, vec);
}

int main(void)
{
	test_mincore(1);
	test_mincore(2);
	test_mincore(DEFAULT_STRLEN);
	test_mincore(DEFAULT_STRLEN + 1);

	puts("+++ exited with 0 +++");
	return 0;
}
