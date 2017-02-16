/*
 * Check decoding of finit_module syscall.
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

#if defined(__NR_finit_module)

# include <stdio.h>
# include <unistd.h>

# include "init_delete_module.h"

int
main(void)
{
	static const kernel_ulong_t bogus_fd =
		(kernel_ulong_t) 0xdeb0d1edbeeff00dULL;

	static const struct {
		kernel_ulong_t val;
		const char *str;
	} flags[] = {
		{ ARG_STR(0) },
		{ (kernel_ulong_t) 0xffffffff00000002ULL,
			"MODULE_INIT_IGNORE_VERMAGIC" },
		{ (kernel_ulong_t) 0xbadc0deddefaced0ULL,
			"0xdefaced0 /* MODULE_INIT_??? */" },
		{ (kernel_ulong_t) 0xfacef157dec0ded1ULL,
			"MODULE_INIT_IGNORE_MODVERSIONS|0xdec0ded0" },
		{ -1LL, "MODULE_INIT_IGNORE_MODVERSIONS|"
			"MODULE_INIT_IGNORE_VERMAGIC|0xfffffffc" },
	};

	long rc;
	char *bogus_param1 = tail_alloc(PARAM1_LEN);
	char *bogus_param2 = tail_alloc(PARAM2_LEN);
	const char *errstr;

	fill_memory_ex(bogus_param1, PARAM1_LEN, PARAM1_BASE, PARAM1_LEN);
	fill_memory_ex(bogus_param2, PARAM2_LEN, PARAM2_BASE, PARAM2_LEN);

	rc = syscall(__NR_finit_module, F8ILL_KULONG_MASK, NULL,
		     F8ILL_KULONG_MASK);
	printf("finit_module(0, NULL, 0) = %s\n", sprintrc(rc));

	rc = syscall(__NR_finit_module, bogus_fd, bogus_param1, flags[0].val);
	errstr = sprintrc(rc);

	printf("finit_module(%d, \"", (int) bogus_fd);
	print_str(PARAM1_BASE, MAX_STRLEN, false);
	printf("\"..., %s) = %s\n", flags[0].str, errstr);

	bogus_param1[PARAM1_LEN - 1] = '\0';

	rc = syscall(__NR_finit_module, bogus_fd, bogus_param1, flags[1].val);
	errstr = sprintrc(rc);

	printf("finit_module(%d, \"", (int) bogus_fd);
	print_str(PARAM1_BASE, MAX_STRLEN, false);
	printf("\", %s) = %s\n", flags[1].str, errstr);

	rc = syscall(__NR_finit_module, bogus_fd, bogus_param2 + PARAM2_LEN,
		flags[2].val);
	printf("finit_module(%d, %p, %s) = %s\n",
	       (int) bogus_fd, bogus_param2 + PARAM2_LEN, flags[2].str,
	       sprintrc(rc));

	rc = syscall(__NR_finit_module, bogus_fd, bogus_param2, flags[3].val);
	printf("finit_module(%d, %p, %s) = %s\n",
	       (int) bogus_fd, bogus_param2, flags[3].str, sprintrc(rc));

	bogus_param2[PARAM2_LEN - 1] = '\0';

	rc = syscall(__NR_finit_module, bogus_fd, bogus_param2, flags[4].val);
	errstr = sprintrc(rc);

	printf("finit_module(%d, \"", (int) bogus_fd);
	print_str(PARAM2_BASE, PARAM2_LEN - 1, true);
	printf("\", %s) = %s\n", flags[4].str, errstr);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_finit_module");

#endif
