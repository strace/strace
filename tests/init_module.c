/*
 * Check decoding of init_module syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

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
