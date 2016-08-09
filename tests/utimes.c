/*
 * Check decoding of utimes syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef __NR_utimes

# include <stdio.h>
# include <sys/time.h>
# include <unistd.h>

#define CAST_NUM(n)						\
	(sizeof(n) == sizeof(long) ?				\
		(unsigned long long) (unsigned long) (n) :	\
		(unsigned long long) (n))

int
main(void)
{
	struct timeval tv;
	if (gettimeofday(&tv, NULL))
		perror_msg_and_fail("gettimeofday");

	static const char sample[] = "utimes_sample";

	long rc = syscall(__NR_utimes, sample, 0);
	printf("utimes(\"%s\", NULL) = %ld %s (%m)\n",
	       sample, rc, errno2name());

	struct timeval *const ts = tail_alloc(sizeof(*ts) * 2);

	rc = syscall(__NR_utimes, 0, ts + 1);
	printf("utimes(NULL, %p) = %ld %s (%m)\n",
	       ts + 1, rc, errno2name());

	ts[0].tv_sec = tv.tv_sec;
	ts[0].tv_usec = tv.tv_usec;
	ts[1].tv_sec = tv.tv_sec - 1;
	ts[1].tv_usec = tv.tv_usec + 1;

	rc = syscall(__NR_utimes, "", ts);
	printf("utimes(\"\", [{%llu, %llu}, {%llu, %llu}])"
	       " = %ld %s (%m)\n",
	       CAST_NUM(ts[0].tv_sec), CAST_NUM(ts[0].tv_usec),
	       CAST_NUM(ts[1].tv_sec), CAST_NUM(ts[1].tv_usec),
	       rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_utimes")

#endif
