/*
 * Check decoding of lookup_dcookie syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>

static void
do_lookup_cookie(uint64_t cookie, char *buf, kernel_ulong_t len)
{
	long rc;
	const char *errstr;

#if (LONG_MAX > INT_MAX) \
  || (defined __x86_64__ && defined __ILP32__) \
  || defined LINUX_MIPSN32
	rc = syscall(__NR_lookup_dcookie, cookie, buf, len);
#else
	rc = syscall(__NR_lookup_dcookie, LL_VAL_TO_PAIR(cookie), buf, len);
#endif

	errstr = sprintrc(rc);
	printf("lookup_dcookie(%" PRIu64 ", ", cookie);

	/* Here, we trust successful return code */
	if ((rc >= 0) && (rc < (long) INT_MAX)) {
		printf("%.*s, ", (int) rc, buf);
	} else {
		if (buf != NULL)
			printf("%p, ", buf);
		else
			printf("NULL, ");
	}

	printf("%" PRIu64 ") = %s\n", (uint64_t) len, errstr);
}

int
main(void)
{
	enum { BUF_SIZE = 4096 };

	static const uint64_t bogus_cookie =
		(uint64_t) 0xf157feeddeadfaceULL;
	static const kernel_ulong_t bogus_len =
		(kernel_ulong_t) 0xbadc0dedda7a1057ULL;

	char *buf = tail_alloc(BUF_SIZE);

	do_lookup_cookie(0, NULL, 0);
	do_lookup_cookie(bogus_cookie, buf + BUF_SIZE, bogus_len);
	do_lookup_cookie(bogus_cookie, buf, BUF_SIZE);

	puts("+++ exited with 0 +++");

	return 0;
}
