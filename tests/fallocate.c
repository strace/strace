/*
 * Check decoding of fallocate syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#if defined(__NR_fallocate) && defined(HAVE_FALLOCATE) && HAVE_FALLOCATE

# include <errno.h>
# include <fcntl.h>
# include <stdio.h>

# include "xlat.h"
# include "xlat/falloc_flags.h"

int
main(void)
{
	static const int bogus_fd = 0xbeefface;
	static const int bogus_mode = 0xdeadca75;
	static const off_t bogus_offset = (off_t) 0xbadc0dedda7a1057LLU;
	static const off_t bogus_len = (off_t) 0xbadfaceca7b0d1e5LLU;

	long rc = fallocate(bogus_fd, bogus_mode, bogus_offset, bogus_len);
	/*
	 * Workaround a bug fixed by commit glibc-2.11-346-gde240a0.
	 */
	if (rc > 0) {
		errno = rc;
		rc = -1;
	}
	const char *errstr = sprintrc(rc);

	printf("fallocate(%d, ", bogus_fd);
	printflags(falloc_flags, (unsigned) bogus_mode, "FALLOC_FL_???");
	printf(", %lld, %lld) = %s\n",
	       (long long) bogus_offset, (long long) bogus_len, errstr);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fallocate && HAVE_FALLOCATE");

#endif
