/*
 * Check decoding of futimesat syscall.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef __NR_futimesat

# include <stdint.h>
# include <stdio.h>
# include <sys/time.h>
# include <unistd.h>


int
main(void)
{
	static const char sample[] = "futimesat_sample";
	unsigned long dirfd = (unsigned long) 0xdeadbeef00000000ULL | -100U;

	long rc = syscall(__NR_futimesat, dirfd, sample, 0);
	printf("futimesat(AT_FDCWD, \"%s\", NULL) = %s\n",
	       sample, sprintrc(rc));

	struct timeval *const ts = tail_alloc(sizeof(*ts) * 2);
	dirfd = (unsigned long) 0xdeadbeefffffffffULL;

	ts[0].tv_sec = 1492356708;
	ts[0].tv_usec = 567891234;
	ts[1].tv_sec = 1492357086;
	ts[1].tv_usec = 678902345;

	rc = syscall(__NR_futimesat, dirfd, 0, ts + 2);
	printf("futimesat(%d, NULL, %p) = %s\n",
	       (int) dirfd, ts + 2, sprintrc(rc));

	rc = syscall(__NR_futimesat, dirfd, 0, ts + 1);
	printf("futimesat(%d, NULL, %p) = %s\n",
	       (int) dirfd, ts + 1, sprintrc(rc));

	(void) close(0);
	rc = syscall(__NR_futimesat, 0, "", ts);
	printf("futimesat(0, \"\", [{tv_sec=%jd, tv_usec=%jd}, "
	       "{tv_sec=%jd, tv_usec=%jd}]) = %s\n",
	       (intmax_t) ts[0].tv_sec, (intmax_t) ts[0].tv_usec,
	       (intmax_t) ts[1].tv_sec, (intmax_t) ts[1].tv_usec,
	       sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_futimesat")

#endif
