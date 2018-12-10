/*
 * Check decoding of set_mempolicy syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_set_mempolicy

# include <errno.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/policies.h"

# define MAX_STRLEN 3
# define NLONGS(n) ((n + 8 * sizeof(long) - 2) \
		      / (8 * sizeof(long)))

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

	long rc = syscall(__NR_set_mempolicy, 0, nodemask, maxnode);
	const char *errstr = sprintrc(rc);

	fputs("set_mempolicy(MPOL_DEFAULT, ", stdout);

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
	if (syscall(__NR_set_mempolicy, 0, 0, 0))
		perror_msg_and_skip("set_mempolicy");
	puts("set_mempolicy(MPOL_DEFAULT, NULL, 0) = 0");

	const unsigned long *nodemask = (void *) 0xfacefeedfffffffeULL;
	const unsigned long maxnode = (unsigned long) 0xcafef00dbadc0dedULL;
	long rc = syscall(__NR_set_mempolicy, 1, nodemask, maxnode);
	printf("set_mempolicy(MPOL_PREFERRED, %p, %lu) = %s\n",
	       nodemask, maxnode, sprintrc(rc));

	test_offset(0);
	test_offset(1);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_set_mempolicy")

#endif
