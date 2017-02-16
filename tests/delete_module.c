/*
 * Check decoding of delete_module syscall.
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

#if defined(__NR_delete_module)

# include <fcntl.h>
# include <stdio.h>
# include <unistd.h>

# include "init_delete_module.h"

int
main(void)
{
	static const struct {
		kernel_ulong_t val;
		const char *str;
		unsigned int val_prefix, val_suffix;
	} flags[] = {
		{ ARG_STR(0), 0, 0 },
		{ F8ILL_KULONG_MASK | O_NONBLOCK, "O_NONBLOCK", 0, 0 },
		{ (kernel_ulong_t) 0xbadc0dedfacef157ULL & ~(O_NONBLOCK | O_TRUNC),
			" /* O_??? */", 0xfacef157U & ~(O_NONBLOCK | O_TRUNC), 0},
		{ (kernel_ulong_t) (0xfacef157deade71cULL & ~O_NONBLOCK) | O_TRUNC,
			"O_TRUNC", 0, 0xdeade71c & ~(O_NONBLOCK | O_TRUNC)},
		{ -1LL, "O_NONBLOCK|O_TRUNC", 0, -1U & ~(O_NONBLOCK | O_TRUNC)},
	};

	long rc;
	char *bogus_param1 = tail_alloc(PARAM1_LEN);
	char *bogus_param2 = tail_alloc(PARAM2_LEN);
	const char *errstr;

	fill_memory_ex(bogus_param1, PARAM1_LEN, PARAM1_BASE, PARAM1_LEN);
	fill_memory_ex(bogus_param2, PARAM2_LEN, PARAM2_BASE, PARAM2_LEN);

	rc = syscall(__NR_delete_module, NULL, F8ILL_KULONG_MASK);
	printf("delete_module(NULL, 0) = %s\n", sprintrc(rc));

	rc = syscall(__NR_delete_module, bogus_param1, flags[0].val);
	errstr = sprintrc(rc);

	printf("delete_module(\"");
	print_str(PARAM1_BASE, MAX_STRLEN, false);
	printf("\"..., %s) = %s\n", flags[0].str, errstr);

	bogus_param1[PARAM1_LEN - 1] = '\0';

	rc = syscall(__NR_delete_module, bogus_param1, flags[1].val);
	errstr = sprintrc(rc);

	printf("delete_module(\"");
	print_str(PARAM1_BASE, MAX_STRLEN, false);
	printf("\", %s) = %s\n", flags[1].str, errstr);

	rc = syscall(__NR_delete_module, bogus_param2 + PARAM2_LEN,
		flags[2].val);
	printf("delete_module(%p, %#x%s) = %s\n",
	       bogus_param2 + PARAM2_LEN, flags[2].val_prefix,
	       flags[2].str, sprintrc(rc));

	rc = syscall(__NR_delete_module, bogus_param2, flags[3].val);
	printf("delete_module(%p, %s|%#x) = %s\n",
	       bogus_param2, flags[3].str, flags[3].val_suffix, sprintrc(rc));

	bogus_param2[PARAM2_LEN - 1] = '\0';

	rc = syscall(__NR_delete_module, bogus_param2, flags[4].val);
	errstr = sprintrc(rc);

	printf("delete_module(\"");
	print_str(PARAM2_BASE, PARAM2_LEN - 1, true);
	printf("\", %s|%#x) = %s\n", flags[4].str, flags[4].val_suffix, errstr);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_delete_module");

#endif
