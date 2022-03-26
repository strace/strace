/*
 * Check decoding of finit_module syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

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
			"MODULE_INIT_IGNORE_VERMAGIC|"
			"MODULE_INIT_COMPRESSED_FILE|0xfffffff8" },
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
