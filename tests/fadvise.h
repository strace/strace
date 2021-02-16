/*
 * Common definitions for fadvise64 and fadvise64_64 tests.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_FADVISE_H
# define STRACE_TESTS_FADVISE_H

# include <limits.h>
# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/advise.h"

static void do_fadvise(long fd, long long offset, long long llen, long advice);

int
main(void)
{
	static const long bogus_fd = (long) 0xfeedf00dbeeffaceULL;
	static const long long bogus_offset = 0xbadc0dedda7a1057ULL;
	static const long long bogus_len = 0xbadfaceca7b0d1e5ULL;
	static const long bogus_advice = (long) 0xf00dfeeddeadca75ULL;

	do_fadvise(bogus_fd, bogus_offset, bogus_len, bogus_advice);

	puts("+++ exited with 0 +++");
	return 0;
}

#endif /* !STRACE_TESTS_FADVISE_H */
