/*
 * Check decoding of mbind syscall.
 *
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
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
k_mbind(const unsigned long start,
	const unsigned long len,
	const unsigned long mode,
	const unsigned long nmask,
	const unsigned long maxnode,
	const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t arg1 = start;
	const kernel_ulong_t arg2 = len;
	const kernel_ulong_t arg3 = mode;
	const kernel_ulong_t arg4 = nmask;
	const kernel_ulong_t arg5 = maxnode;
	const kernel_ulong_t arg6 = fill | flags;
	const long rc = syscall(__NR_mbind, arg1, arg2, arg3, arg4, arg5, arg6);
	errstr = sprintrc(rc);
	return rc;
}

#if XLAT_RAW
# define out_str	raw
# define flags_str	"0xffffffff"
#elif XLAT_VERBOSE
# define out_str	verbose
# define flags_str	"0xffffffff /* MPOL_MF_STRICT|MPOL_MF_MOVE" \
			"|MPOL_MF_MOVE_ALL|0xfffffff8 */"
#else
# define out_str	abbrev
# define flags_str	"MPOL_MF_STRICT|MPOL_MF_MOVE|MPOL_MF_MOVE_ALL|0xfffffff8"
#endif

static struct {
	unsigned long val;
	const char *raw;
	const char *verbose;
	const char *abbrev;
} mpol_modes[] = {
	{ ARG_STR(0),
	  "0 /* MPOL_DEFAULT */",
	  "MPOL_DEFAULT" },
	{ ARG_STR(0x1),
	  "0x1 /* MPOL_PREFERRED */",
	  "MPOL_PREFERRED" },
	{ ARG_STR(0x2),
	  "0x2 /* MPOL_BIND */",
	  "MPOL_BIND" },
	{ ARG_STR(0x3),
	  "0x3 /* MPOL_INTERLEAVE */",
	  "MPOL_INTERLEAVE" },
	{ ARG_STR(0x4),
	  "0x4 /* MPOL_LOCAL */",
	  "MPOL_LOCAL" },
	{ ARG_STR(0x5),
	  "0x5 /* MPOL_PREFERRED_MANY */",
	  "MPOL_PREFERRED_MANY" },
	{ ARG_STR(0x8000),
	  "0x8000 /* MPOL_DEFAULT|MPOL_F_STATIC_NODES */",
	  "MPOL_DEFAULT|MPOL_F_STATIC_NODES" },
	{ ARG_STR(0x4001),
	  "0x4001 /* MPOL_PREFERRED|MPOL_F_RELATIVE_NODES */",
	  "MPOL_PREFERRED|MPOL_F_RELATIVE_NODES" },
	{ ARG_STR(0x2002),
	  "0x2002 /* MPOL_BIND|MPOL_F_NUMA_BALANCING */",
	  "MPOL_BIND|MPOL_F_NUMA_BALANCING" },
	{ ARG_STR(0xe003),
	  "0xe003 /* MPOL_INTERLEAVE|MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES"
		"|MPOL_F_NUMA_BALANCING */",
	  "MPOL_INTERLEAVE|MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES"
		"|MPOL_F_NUMA_BALANCING" },
	{ ARG_STR(0x6),
	  "0x6 /* MPOL_??? */",
	  "0x6 /* MPOL_??? */" },
	{ ARG_STR(0xffff1fff),
	  "0xffff1fff /* MPOL_??? */",
	  "0xffff1fff /* MPOL_??? */" },
	{ ARG_STR(0xffffffff),
	  "0xffffffff /* MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES"
		"|MPOL_F_NUMA_BALANCING|0xffff1fff */",
	  "MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES|MPOL_F_NUMA_BALANCING"
		"|0xffff1fff" },
#if SIZEOF_LONG > 4
	{ 0xffffffff00000000UL,
	  "0xffffffff00000000",
	  "0xffffffff00000000 /* MPOL_??? */",
	  "0xffffffff00000000 /* MPOL_??? */" },
	{ 0xffffffffffff1fffUL,
	  "0xffffffffffff1fff",
	  "0xffffffffffff1fff /* MPOL_??? */",
	  "0xffffffffffff1fff /* MPOL_??? */" },
	{ -1UL,
	  "0xffffffffffffffff",
	  "0xffffffffffffffff /* MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES"
		"|MPOL_F_NUMA_BALANCING|0xffffffffffff1fff */",
	  "MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES|MPOL_F_NUMA_BALANCING"
		"|0xffffffffffff1fff" },
#endif
};

int
main(void)
{
	const unsigned long size = get_page_size();
	unsigned long *const addr = tail_alloc(size);
	const unsigned long start = (unsigned long) 0xfffffff1fffffff2ULL;
	const unsigned long len = (unsigned long) 0xfffffff4fffffff4ULL;
	const unsigned long nodemask = (unsigned long) 0xfffffff5fffffff6ULL;
	const unsigned long maxnode = (unsigned long) 0xfffffff7fffffff8ULL;

	if (k_mbind((unsigned long) addr, size, mpol_modes[0].val, 0, 0, 0))
		perror_msg_and_skip("mbind");
	printf("mbind(%p, %lu, %s, NULL, 0, 0) = 0\n",
	       addr, size, mpol_modes[0].out_str);

	for (unsigned int i = 0; i < ARRAY_SIZE(mpol_modes); ++i) {
		if (i) {
			k_mbind((unsigned long) addr, size, mpol_modes[i].val,
				0, 0, 0);
			printf("mbind(%p, %lu, %s, NULL, 0, 0) = %s\n",
			       addr, size, mpol_modes[i].out_str, errstr);
		}

		k_mbind(start, len, mpol_modes[i].val,
			nodemask, maxnode, -1U);
		printf("mbind(%#lx, %lu, %s, %#lx, %lu, %s) = %s\n",
		       start, len, mpol_modes[i].out_str,
		       nodemask, maxnode, flags_str, errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
