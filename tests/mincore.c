/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
#include <stdio.h>
#include <sys/mman.h>

#define DEFAULT_STRLEN 32

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
