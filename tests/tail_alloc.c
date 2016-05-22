/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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
