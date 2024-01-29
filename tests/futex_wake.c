/*
 * Check decoding of futex_wake syscall.
 *
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "futex2_flags.h"
#include "scno.h"
#include "xmalloc.h"

#include <stdio.h>
#include <unistd.h>

static const char *errstr;

static long
k_futex_wake(const void *const uaddr,
	     const unsigned long mask,
	     const unsigned int nr,
	     const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = (uintptr_t) uaddr;
	const kernel_ulong_t arg2 = mask;
	const kernel_ulong_t arg3 = fill | nr;
	const kernel_ulong_t arg4 = fill | flags;
	const long rc = syscall(__NR_futex_wake, arg1, arg2, arg3, arg4, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

static const struct {
	unsigned long val;
	const char *raw;
	const char *abbrev;
	const char *verbose;
} masks[] = { {
		ARG_STR(0),
		"0",
		"0"
	}, {
		ARG_STR(0xffffffff),
		"FUTEX_BITSET_MATCH_ANY",
		"0xffffffff /* FUTEX_BITSET_MATCH_ANY */"
	}, {
#if SIZEOF_LONG == 4
		ARG_STR(0xfffffff1),
		"0xfffffff1",
		"0xfffffff1"
#else
		0xfffffff1fffffff2UL,
		"0xfffffff1fffffff2",
		"0xfffffff1fffffff2",
		"0xfffffff1fffffff2"
#endif
	},
};

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, uaddr);
	const int nr = 0xfacefeed;

	struct {
		void *val;
		const char *str;
	} addrs[] = {
		{ ARG_STR(NULL) },
		{ uaddr, xasprintf("%p", uaddr) },
	};

	for (size_t a = 0; a < ARRAY_SIZE(addrs); ++a) {
		for (size_t m = 0; m < ARRAY_SIZE(masks); ++m) {
			for (size_t f = 0; f < ARRAY_SIZE(futex2_flags); ++f) {
				k_futex_wake(addrs[a].val,
					     masks[m].val,
					     nr,
					     futex2_flags[f].val);
				printf("futex_wake(%s, %s, %d, %s) = %s\n",
				       addrs[a].str,
				       masks[m].XLAT_name,
				       nr,
				       futex2_flags[f].XLAT_name,
				       errstr);
			}
		}
	}

	puts("+++ exited with 0 +++");
	return 0;
}
