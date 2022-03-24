/*
 * Check decoding of set_mempolicy_home_node syscall.
 *
 * Copyright (c) 2022 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

static const char *errstr;

static long
sys_set_mempolicy_home_node(kernel_ulong_t start, kernel_ulong_t len,
			    kernel_ulong_t home_node, kernel_ulong_t flags)
{
	static const kernel_ulong_t fill =
		(kernel_ulong_t) 0xbadc0ded00000000ULL;
	kernel_ulong_t arg1 = start;
	kernel_ulong_t arg2 = len;
	kernel_ulong_t arg3 = home_node;
	kernel_ulong_t arg4 = flags;
	kernel_ulong_t arg5 = fill | 0xdecaffed;
	kernel_ulong_t arg6 = fill | 0xdeefaced;

	long rc = syscall(__NR_set_mempolicy_home_node,
			  arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

#define KUL_1 ((unsigned long long) (kernel_ulong_t) -1ULL)

int
main(void)
{
	void *dummy;

	sys_set_mempolicy_home_node(0, 0, 0, 0);
	printf("set_mempolicy_home_node(NULL, 0, 0, 0) = %s\n", errstr);

	sys_set_mempolicy_home_node((uintptr_t) &dummy, 1, 2, 3);
	printf("set_mempolicy_home_node(%p, 1, 2, 0x3) = %s\n", &dummy, errstr);

	sys_set_mempolicy_home_node(-1, -2, -3, -4);
	printf("set_mempolicy_home_node(%#llx, %llu, %llu, %#llx) = %s\n",
	       KUL_1, KUL_1 - 1, KUL_1 - 2, KUL_1 - 3, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
