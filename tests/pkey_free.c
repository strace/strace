/*
 * Check decoding of pkey_free syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_pkey_free

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	static const kernel_ulong_t keys[] = {
		0,
		3141592653U,
		(kernel_ulong_t) 0xbadc0ded00000000ULL,
		(kernel_ulong_t) 0xffff00001111eeeeULL,
		(kernel_ulong_t) 0x123456789abcdef0ULL,
	};

	long rc;
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(keys); i++) {
		rc = syscall(__NR_pkey_free, keys[i]);
		printf("pkey_free(%d) = %s\n", (int) keys[i], sprintrc(rc));
	}

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_pkey_free");

#endif
