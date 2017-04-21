/*
 * Check decoding of utime syscall.
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

#ifdef __NR_utime

# include <time.h>
# include <utime.h>
# include <errno.h>
# include <stdio.h>
# include <unistd.h>

static long
k_utime(const void *const filename, const void *const times)
{
	return syscall(__NR_utime, filename, times);
}

int
main(void)
{
	static const char *const dummy_str = "dummy filename";

	const time_t t = 1492350678;
	const struct utimbuf u = { .actime = t, .modtime = t };
	const struct utimbuf *const tail_u = tail_memdup(&u, sizeof(u));
	const char *const dummy_filename =
		tail_memdup(dummy_str, sizeof(dummy_str) - 1);

	long rc = k_utime("", NULL);
	printf("utime(\"\", NULL) = %s\n", sprintrc(rc));

	rc = k_utime(dummy_filename + sizeof(dummy_str), tail_u + 1);
	printf("utime(%p, %p) = %s\n", dummy_filename + sizeof(dummy_str),
	       tail_u + 1, sprintrc(rc));

	rc = k_utime(dummy_filename, (struct tm *) tail_u + 1);
	printf("utime(%p, %p) = %s\n",
	       dummy_filename, (struct tm *) tail_u + 1, sprintrc(rc));

	rc = k_utime("utime\nfilename", tail_u);
	const char *errstr = sprintrc(rc);
	printf("utime(\"utime\\nfilename\", {actime=%lld", (long long) t);
	print_time_t_nsec(t, 0, 1);
	printf(", modtime=%lld", (long long) t);
	print_time_t_nsec(t, 0, 1);
	printf("}) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_utime")

#endif
