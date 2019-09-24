/*
 * This file is part of splice strace test.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_splice

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd_in = (long int) 0xdeadbeefffffffffULL;
	const long int fd_out = (long int) 0xdeadbeeffffffffeULL;
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_in);
	TAIL_ALLOC_OBJECT_CONST_PTR(long long int, off_out);
	*off_in = 0xdeadbef1facefed1ULL;
	*off_out = 0xdeadbef2facefed2ULL;
	const size_t len = (size_t) 0xdeadbef3facefed3ULL;
	const unsigned int flags = 15;

	long rc = syscall(__NR_splice,
			  fd_in, off_in, fd_out, off_out, len, flags);
	printf("splice(%d, [%lld], %d, [%lld], %zu, %s) = %ld %s (%m)\n",
	       (int) fd_in, *off_in, (int) fd_out, *off_out, len,
	       "SPLICE_F_MOVE|SPLICE_F_NONBLOCK|SPLICE_F_MORE|SPLICE_F_GIFT",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_splice")

#endif
