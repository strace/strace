/*
 * Check decoding of init_module syscall.
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

#if defined(__NR_init_module)

# include <stdio.h>
# include <unistd.h>

# include "init_delete_module.h"

int
main(void)
{

	static const kernel_ulong_t bogus_addr =
		(kernel_ulong_t) 0xfffffeedfffffaceULL;
	static const kernel_ulong_t bogus_len =
		(kernel_ulong_t) 0xfffffca7ffffc0deULL;

	long rc;
	char *bogus_param1 = tail_alloc(PARAM1_LEN);
	char *bogus_param2 = tail_alloc(PARAM2_LEN);
	const char *errstr;

	fill_memory_ex(bogus_param1, PARAM1_LEN, PARAM1_BASE, PARAM1_LEN);
	fill_memory_ex(bogus_param2, PARAM2_LEN, PARAM2_BASE, PARAM2_LEN);

	rc = syscall(__NR_init_module, NULL, F8ILL_KULONG_MASK, NULL);
	printf("init_module(NULL, %llu, NULL) = %s\n",
	       (unsigned long long) F8ILL_KULONG_MASK, sprintrc(rc));

	rc = syscall(__NR_init_module, bogus_addr, 0, bogus_param1);
	errstr = sprintrc(rc);

	printf("init_module(%#llx, 0, \"", (unsigned long long) bogus_addr);
	print_str(PARAM1_BASE, MAX_STRLEN, false);
	printf("\"...) = %s\n", errstr);

	bogus_param1[PARAM1_LEN - 1] = '\0';

	rc = syscall(__NR_init_module, bogus_addr, 0, bogus_param1);
	errstr = sprintrc(rc);

	printf("init_module(%#llx, 0, \"", (unsigned long long) bogus_addr);
	print_str(PARAM1_BASE, MAX_STRLEN, false);
	printf("\") = %s\n", errstr);

	rc = syscall(__NR_init_module, bogus_addr, bogus_len,
		bogus_param2 + PARAM2_LEN);
	printf("init_module(%#llx, %llu, %p) = %s\n",
	       (unsigned long long) bogus_addr, (unsigned long long) bogus_len,
	       bogus_param2 + PARAM2_LEN, sprintrc(rc));

	rc = syscall(__NR_init_module, NULL, bogus_len, bogus_param2);
	printf("init_module(NULL, %llu, %p) = %s\n",
	       (unsigned long long) bogus_len, bogus_param2, sprintrc(rc));

	bogus_param2[PARAM2_LEN - 1] = '\0';

	rc = syscall(__NR_init_module, NULL, bogus_len, bogus_param2);
	errstr = sprintrc(rc);

	printf("init_module(NULL, %llu, \"", (unsigned long long) bogus_len);
	print_str(PARAM2_BASE, PARAM2_LEN - 1, true);
	printf("\") = %s\n", errstr);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_init_module");

#endif
