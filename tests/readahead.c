/*
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef HAVE_READAHEAD
/* Check for glibc readahead argument passing bugs. */
/*
 * glibc < 2.8 had an incorrect order of higher and lower parts of offset,
 * see https://sourceware.org/bugzilla/show_bug.cgi?id=5208
 */
# if GLIBC_PREREQ_LT(2, 8)
#  undef HAVE_READAHEAD
# endif /* glibc < 2.8 */
/*
 * glibc < 2.25 had an incorrect implementation on mips n64,
 * see https://sourceware.org/bugzilla/show_bug.cgi?id=21026
 */
# if GLIBC_PREREQ_LT(2, 25) && defined LINUX_MIPSN64
#  undef HAVE_READAHEAD
# endif /* LINUX_MIPSN64 && glibc < 2.25 */
#endif /* HAVE_READAHEAD */

#ifdef HAVE_READAHEAD

# include <fcntl.h>
# include <stdio.h>

static const int fds[] = {
	-0x80000000,
	-100,
	-1,
	0,
	1,
	2,
	0x7fffffff,
};

static const off64_t offsets[] = {
	-0x8000000000000000LL,
	-0x5060708090a0b0c0LL,
	-1LL,
	 0,
	 1,
	 0xbadfaced,
	 0x7fffffffffffffffLL,
};

static const unsigned long counts[] = {
	0UL,
	0xdeadca75,
	(unsigned long) 0xface1e55beeff00dULL,
	(unsigned long) 0xffffffffffffffffULL,
};

int
main(void)
{
	unsigned i;
	unsigned j;
	unsigned k;
	ssize_t rc;

	for (i = 0; i < ARRAY_SIZE(fds); i++)
		for (j = 0; j < ARRAY_SIZE(offsets); j++)
			for (k = 0; k < ARRAY_SIZE(counts); k++) {
				rc = readahead(fds[i], offsets[j], counts[k]);

				printf("readahead(%d, %lld, %lu) = %s\n",
					fds[i], (long long) offsets[j],
					counts[k], sprintrc(rc));
			}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_READAHEAD")

#endif
