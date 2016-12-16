/*
 * Check decoding of lookup_dcookie syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

#include <asm/unistd.h>

#ifdef __NR_lookup_dcookie

# include <inttypes.h>
# include <limits.h>
# include <stdio.h>
# include <unistd.h>

static void
do_lookup_cookie(uint64_t cookie, char *buf, kernel_ulong_t len)
{
	long rc;
	const char *errstr;

# if (LONG_MAX > INT_MAX) \
  || (defined __x86_64__ && defined __ILP32__) \
  || defined LINUX_MIPSN32
	rc = syscall(__NR_lookup_dcookie, cookie, buf, len);
# else
	rc = syscall(__NR_lookup_dcookie, LL_VAL_TO_PAIR(cookie), buf, len);
# endif

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

#else

SKIP_MAIN_UNDEFINED("__NR_lookup_dcookie");

#endif
