/*
 * Check decoding of set_mempolicy syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAX_STRLEN 3
#define NLONGS(n) ((n + 8 * sizeof(long) - 2) \
		      / (8 * sizeof(long)))

static const char *errstr;

static long
k_set_mempolicy(const unsigned int mode,
		const void *const nmask,
		const unsigned long maxnode)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | mode;
	const kernel_ulong_t arg2 = (unsigned long) nmask;
	const kernel_ulong_t arg3 = maxnode;
	const long rc = syscall(__NR_set_mempolicy,
				arg1, arg2, arg3, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

#if XLAT_RAW
# define out_str	raw
#elif XLAT_VERBOSE
# define out_str	verbose
#else
# define out_str	abbrev
#endif

static struct {
	unsigned int val;
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
	  "MPOL_F_STATIC_NODES|MPOL_F_RELATIVE_NODES|MPOL_F_NUMA_BALANCING|0xffff1fff" }
};

static void
print_nodes(const unsigned long maxnode, unsigned int offset)
{
	unsigned int nlongs = NLONGS(maxnode);
	if (nlongs <= offset)
		nlongs = 0;
	else
		nlongs -= offset;
	const unsigned int size = nlongs * sizeof(long);
	unsigned long *const nodemask =
		tail_alloc(size ? size : (offset ? 1 : 0));
	memset(nodemask, 0, size);

	k_set_mempolicy(mpol_modes[0].val, nodemask, maxnode);

	printf("set_mempolicy(%s, ", mpol_modes[0].out_str);

	if (nlongs) {
		putc('[', stdout);
		unsigned int i;
		for (i = 0; i < nlongs + offset; ++i) {
			if (i)
				fputs(", ", stdout);
			if (i < nlongs) {
				if (i >= MAX_STRLEN) {
					fputs("...", stdout);
					break;
				}
				printf("%#0*lx", (int) sizeof(long) * 2 + 2,
				       nodemask[i]);
			} else {
				printf("... /* %p */", nodemask + i);
				break;
			}
		}
		putc(']', stdout);
	} else {
		if (maxnode)
			printf("%p", nodemask);
		else
			printf("[]");
	}

	printf(", %lu) = %s\n", maxnode, errstr);
}

static void
test_offset(const unsigned int offset)
{
	unsigned long maxnode = get_page_size() * 8;

	print_nodes(maxnode, offset);
	print_nodes(maxnode + 1, offset);
	print_nodes(maxnode + 2, offset);

	maxnode = sizeof(long) * 8;
	print_nodes(0, offset);
	print_nodes(1, offset);
	print_nodes(2, offset);
	print_nodes(maxnode - 1, offset);
	print_nodes(maxnode    , offset);
	print_nodes(maxnode + 1, offset);
	print_nodes(maxnode + 2, offset);
	print_nodes(maxnode * 2 - 1, offset);
	print_nodes(maxnode * 2    , offset);
	print_nodes(maxnode * 2 + 1, offset);
	print_nodes(maxnode * 2 + 2, offset);
	print_nodes(maxnode * 3 - 1, offset);
	print_nodes(maxnode * 3    , offset);
	print_nodes(maxnode * 3 + 1, offset);
	print_nodes(maxnode * 3 + 2, offset);
	print_nodes(maxnode * 4 + 2, offset);
}

int
main(void)
{
	if (k_set_mempolicy(mpol_modes[0].val, 0, 0))
		perror_msg_and_skip("set_mempolicy");
	printf("set_mempolicy(%s, NULL, 0) = 0\n", mpol_modes[0].out_str);

	const unsigned long *nodemask = (void *) 0xfacefeedfffffffeULL;
	const unsigned long maxnode = (unsigned long) 0xcafef00ddeadbeefULL;

	for (unsigned int i = 0; i < ARRAY_SIZE(mpol_modes); ++i) {
		if (i) {
			k_set_mempolicy(mpol_modes[i].val, 0, 0);
			printf("set_mempolicy(%s, NULL, 0) = %s\n",
			       mpol_modes[i].out_str, errstr);
		}

		k_set_mempolicy(mpol_modes[i].val, nodemask, maxnode);
		printf("set_mempolicy(%s, %p, %lu) = %s\n",
		       mpol_modes[i].out_str, nodemask, maxnode, errstr);
	}

	test_offset(0);
	test_offset(1);

	puts("+++ exited with 0 +++");
	return 0;
}
