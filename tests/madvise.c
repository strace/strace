/*
 * Copyright (c) 2017 The strace developers.
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
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include <asm/unistd.h>

static const char *errstr;

static long
k_madvise(const kernel_ulong_t addr,
	  const kernel_ulong_t length,
	  const kernel_ulong_t advice)
{
	long rc = syscall(__NR_madvise, addr, length, advice);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	const unsigned long length = get_page_size();
	void *const addr = tail_alloc(length);
	long rc;

	rc = madvise(addr, length, MADV_NORMAL);
	printf("madvise(%p, %lu, MADV_NORMAL) = %s\n",
	       addr, length, sprintrc(rc));

	static const kernel_ulong_t advice =
		(kernel_ulong_t) 0xfacefeed00000000ULL | MADV_RANDOM;
	rc = k_madvise((uintptr_t) addr, length, advice);
	printf("madvise(%p, %lu, MADV_RANDOM) = %s\n",
	       addr, length, sprintrc(rc));

	static const kernel_ulong_t bogus_length =
		(kernel_ulong_t) 0xfffffffffffffaceULL;
	rc = k_madvise(0, bogus_length, MADV_SEQUENTIAL);
	printf("madvise(NULL, %llu, MADV_SEQUENTIAL) = %s\n",
	       (unsigned long long) bogus_length, sprintrc(rc));

	if (F8ILL_KULONG_SUPPORTED) {
		rc = k_madvise(f8ill_ptr_to_kulong(addr), length, MADV_NORMAL);
		printf("madvise(%#llx, %lu, MADV_NORMAL) = %s\n",
		       (unsigned long long) f8ill_ptr_to_kulong(addr),
		       length, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}
