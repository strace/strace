/*
 * Check decoding of mbind syscall.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_mbind

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	const unsigned long len = (unsigned long) 0xcafef00dbadc0dedULL;
	const unsigned long mode = 3;
	const unsigned long nodemask = (unsigned long) 0xfacefeedfffffff1ULL;
	const unsigned long maxnode = (unsigned long) 0xdeadbeeffffffff2ULL;
	const unsigned long flags = -1UL;

	long rc = syscall(__NR_mbind, 0, len, mode, nodemask, maxnode, flags);
	printf("mbind(NULL, %lu, %s, %#lx, %lu, %s|%#x) = %ld %s (%m)\n",
	       len, "MPOL_INTERLEAVE", nodemask, maxnode,
	       "MPOL_MF_STRICT|MPOL_MF_MOVE|MPOL_MF_MOVE_ALL",
	       (unsigned) flags & ~7, rc, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_mbind")

#endif
