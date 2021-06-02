/*
 * Check decoding of copy_file_range syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	const long int fd_in = (long int) 0xdeadbeefffffffff;
	const long int fd_out = (long int) 0xdeadbeeffffffffe;
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_in);
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_out);
	*off_in = 0xdeadbef1facefed1;
	*off_out = 0xdeadbef2facefed2;
	const size_t len = (size_t) 0xdeadbef3facefed3ULL;
	const unsigned int flags = 0;

	long rc = syscall(__NR_copy_file_range,
			  fd_in, off_in, fd_out, off_out, len, flags);
	printf("copy_file_range(%d, [%lld], %d, [%lld], %zu, %u)"
	       " = %ld %s (%m)\n",
	       (int) fd_in, *off_in, (int) fd_out, *off_out, len, flags,
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}
