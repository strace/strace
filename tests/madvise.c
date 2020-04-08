/*
 * Copyright (c) 2017-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
#include "scno.h"

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
