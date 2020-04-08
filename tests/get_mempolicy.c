/*
 * Check decoding of get_mempolicy syscall.
 *
 * Copyright (c) 2016-2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_get_mempolicy

# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/mpol_modes.h"

# define MAX_STRLEN 3
# define NLONGS(n) ((n + 8 * sizeof(long) - 2) \
		      / (8 * sizeof(long)))

static void
print_nodes(unsigned long maxnode)
{
	unsigned long *const nodemask =
		tail_alloc(sizeof(*nodemask) * NLONGS(maxnode));

	if (syscall(__NR_get_mempolicy, 0, nodemask, maxnode, 0, 0)) {
		printf("get_mempolicy(NULL, %p, %lu, NULL, 0) = -1 %s (%m)\n",
		       nodemask, maxnode, errno2name());
		return;
	}

	printf("get_mempolicy(NULL, [");

	unsigned int nlongs = NLONGS(maxnode);
	unsigned int i;
	for (i = 0; i < nlongs; ++i) {
		if (i)
			fputs(", ", stdout);
		if (i >= MAX_STRLEN) {
			fputs("...", stdout);
			break;
		}
		printf("%#0*lx", (int) sizeof(*nodemask) * 2 + 2, nodemask[i]);
	}

	printf("], %lu, NULL, 0) = 0\n", maxnode);
}

int
main(void)
{
	long rc;

	if (syscall(__NR_get_mempolicy, 0, 0, 0, 0, 0))
		perror_msg_and_skip("get_mempolicy");
	puts("get_mempolicy(NULL, NULL, 0, NULL, 0) = 0");

	int *mode = (void *) 0xdefaced1baddeed2;
	unsigned long maxnode = (unsigned long) 0xcafef00dbadc0dedULL;
	const unsigned long *nodemask = (void *) 0xfacedad3bebefed4ULL;
	const unsigned long addr = (unsigned long) 0xfacefeeddeadbeefULL;
	const unsigned long flags = -1U;
	rc = syscall(__NR_get_mempolicy, mode, nodemask, maxnode, addr, flags);
	printf("get_mempolicy(%p, %p, %lu, %#lx, %s|%#lx) = %ld %s (%m)\n",
	       mode, nodemask, maxnode, addr,
	       "MPOL_F_NODE|MPOL_F_ADDR|MPOL_F_MEMS_ALLOWED",
	       flags & ~7, rc, errno2name());

	mode = tail_alloc(sizeof(*mode));

	rc = syscall(__NR_get_mempolicy, mode, 0, 0, 0, 0);
	printf("get_mempolicy([");
	printxval(mpol_modes, (unsigned) *mode, "MPOL_???");
	printf("], NULL, 0, NULL, 0) = %ld\n", rc);

	*mode = -1;
	rc = syscall(__NR_get_mempolicy, mode, 0, 0, mode - 1, 2);
	printf("get_mempolicy([");
	printxval(mpol_modes, (unsigned) *mode, "MPOL_???");
	printf("], NULL, 0, %p, MPOL_F_ADDR) = %ld\n", mode - 1, rc);

	maxnode = get_page_size() * 8;

	print_nodes(maxnode);
	print_nodes(maxnode + 1);
	print_nodes(maxnode + 2);

	maxnode = sizeof(*nodemask) * 8;
	print_nodes(maxnode - 1);
	print_nodes(maxnode    );
	print_nodes(maxnode + 1);
	print_nodes(maxnode + 2);
	print_nodes(maxnode * 2 - 1);
	print_nodes(maxnode * 2    );
	print_nodes(maxnode * 2 + 1);
	print_nodes(maxnode * 2 + 2);
	print_nodes(maxnode * 3 - 1);
	print_nodes(maxnode * 3    );
	print_nodes(maxnode * 3 + 1);
	print_nodes(maxnode * 3 + 2);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_get_mempolicy")

#endif
