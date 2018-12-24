/*
 * Check decoding of int 0x80 on x86_64, x32, and x86.
 *
 * Copyright (c) 2017-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#if defined __x86_64__ || defined __i386__

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	/* 200 is __NR_getgid32 on x86 and __NR_tkill on x86_64. */
	__asm__("movl $200, %eax; int $0x80");
	printf("getgid32() = %d\n", getegid());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__x86_64__ || __i386__")

#endif
