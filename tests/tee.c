/*
 * This file is part of tee strace test.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined __NR_tee

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const long int fd_in = (long int) 0xdeadbeefffffffffULL;
	const long int fd_out = (long int) 0xdeadbeeffffffffeULL;
	const size_t len = (size_t) 0xdeadbef3facefed3ULL;
	const unsigned int flags = 15;

	long rc = syscall(__NR_tee, fd_in, fd_out, len, flags);
	printf("tee(%d, %d, %zu, %s) = %ld %s (%m)\n",
	       (int) fd_in, (int) fd_out, len,
	       "SPLICE_F_MOVE|SPLICE_F_NONBLOCK|SPLICE_F_MORE|SPLICE_F_GIFT",
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_tee")

#endif
