/*
 * Check decoding of map_shadow_stack syscall.
 *
 * Copyright (c) 2023-2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static long
k_map_shadow_stack(const unsigned long addr,
		   const unsigned long len,
		   const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = addr;
	const kernel_ulong_t arg2 = len;
	const kernel_ulong_t arg3 = fill | flags;
	const long rc = syscall(__NR_map_shadow_stack, arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static const struct {
	unsigned long val;
	const char *raw;
	const char *verbose;
	const char *abbrev;
} flags[] = { {
		ARG_STR(0x1),
		"0x1 /* SHADOW_STACK_SET_TOKEN */",
		"SHADOW_STACK_SET_TOKEN"
	}, {
		ARG_STR(0xfffffffe),
		"0xfffffffe /* SHADOW_STACK_??? */",
		"0xfffffffe /* SHADOW_STACK_??? */"
	}, {
		ARG_STR(0xffffffff),
		"0xffffffff /* SHADOW_STACK_SET_TOKEN|0xfffffffe */",
		"SHADOW_STACK_SET_TOKEN|0xfffffffe"
	},
};

int
main(void)
{
	const unsigned long addr = (unsigned long) 0xfffffff1fffffff2ULL;
	const unsigned long len = (unsigned long) 0xfffffff3fffffff4ULL;

	k_map_shadow_stack(0, 0, 0);
	printf("map_shadow_stack(NULL, 0, 0) = %s\n", errstr);

	for (size_t i = 0; i < ARRAY_SIZE(flags); ++i) {
		k_map_shadow_stack(addr, len, flags[i].val);
		printf("map_shadow_stack(%#lx, %lu, %s) = %s\n",
		       addr, len, flags[i].XLAT_name, errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
