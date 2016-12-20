/*
 * Check decoding of kcmp syscall.
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
#include "scno.h"

#ifdef __NR_kcmp

# include <stdio.h>
# include <unistd.h>

# define KCMP_FILE     0
# define KCMP_SYSVSEM  6

static void
do_kcmp(kernel_ulong_t pid1, kernel_ulong_t pid2, kernel_ulong_t type,
	const char *type_str, kernel_ulong_t idx1, kernel_ulong_t idx2)
{
	long rc;
	const char *errstr;

	rc = syscall(__NR_kcmp, pid1, pid2, type, idx1, idx2);
	errstr = sprintrc(rc);

	printf("kcmp(%d, %d, ", (int) pid1, (int) pid2);

	if (type_str)
		printf("%s", type_str);
	else
		printf("%#x /* KCMP_??? */", (int) type);

	if (type == KCMP_FILE)
		printf(", %u, %u", (unsigned) idx1, (unsigned) idx2);
	else if (type > KCMP_SYSVSEM)
		printf(", %#llx, %#llx",
		       (unsigned long long) idx1, (unsigned long long) idx2);

	printf(") = %s\n", errstr);
}

int
main(void)
{
	static const kernel_ulong_t bogus_pid1 =
		(kernel_ulong_t) 0xdeadca75face1057ULL;
	static const kernel_ulong_t bogus_pid2 =
		(kernel_ulong_t) 0xdefaced1defaced2ULL;
	static const kernel_ulong_t bogus_type =
		(kernel_ulong_t) 0xbadc0dedda7adeadULL;
	static const kernel_ulong_t bogus_idx1 =
		(kernel_ulong_t) 0xdec0ded3dec0ded4ULL;
	static const kernel_ulong_t bogus_idx2 =
		(kernel_ulong_t) 0xba5e1e55deadc0deULL;

	/* Invalid values */
	do_kcmp(bogus_pid1, bogus_pid2, bogus_type, NULL, bogus_idx1,
		bogus_idx2);
	do_kcmp(0, 0, KCMP_SYSVSEM + 1, NULL, 0, 0);

	/* KCMP_FILE is the only type which has additional args */
	do_kcmp(3141592653U, 2718281828U, ARG_STR(KCMP_FILE), bogus_idx1,
		bogus_idx2);
	/* Some type without additional args */
	do_kcmp(-1, -1, ARG_STR(KCMP_SYSVSEM), bogus_idx1, bogus_idx2);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_kcmp");

#endif
