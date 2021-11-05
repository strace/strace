/*
 * Check decoding of pkey_alloc syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t flags[] = {
		0,
		(kernel_ulong_t) 0xbadc0ded00000000ULL,
		(kernel_ulong_t) 0xffff0000eeee1111ULL,
		(kernel_ulong_t) 0x123456789abcdef0ULL,
	};
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} rights[] = {
		{ (kernel_ulong_t) 0xbadc0ded00000002ULL,
			sizeof(kernel_ulong_t) > sizeof(int) ?
			"PKEY_DISABLE_WRITE|0xbadc0ded00000000" :
			"PKEY_DISABLE_WRITE" },
		{ 0xdec0ded, "PKEY_DISABLE_ACCESS|PKEY_DISABLE_EXECUTE|"
				"0xdec0de8" },
		{ 0x7, "PKEY_DISABLE_ACCESS|PKEY_DISABLE_WRITE|"
				"PKEY_DISABLE_EXECUTE" },
		{ ARG_STR(0) },
		{ 0xbadc0de8, "0xbadc0de8 /* PKEY_??? */" },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(flags); ++i) {
		for (unsigned int j = 0; j < ARRAY_SIZE(rights); ++j) {
			long rc = syscall(__NR_pkey_alloc,
					  flags[i], rights[j].val);
			printf("pkey_alloc(%#llx, %s) = %s\n",
			       (unsigned long long) flags[i], rights[j].str,
			       sprintrc(rc));
		}
	}

	puts("+++ exited with 0 +++");

	return 0;
}
