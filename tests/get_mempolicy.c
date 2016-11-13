/*
 * Check decoding of get_mempolicy syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_get_mempolicy

# include <stdio.h>
# include <unistd.h>

# include "xlat.h"
# include "xlat/policies.h"

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
	       "MPOL_F_NODE|MPOL_F_ADDR",
	       flags & ~3, rc, errno2name());

	mode = tail_alloc(sizeof(*mode));

	rc = syscall(__NR_get_mempolicy, mode, 0, 0, 0, 0);
	printf("get_mempolicy([");
	printxval(policies, (unsigned) *mode, "MPOL_???");
	printf("], NULL, 0, NULL, 0) = %ld\n", rc);

	*mode = -1;
	rc = syscall(__NR_get_mempolicy, mode, 0, 0, mode - 1, 2);
	printf("get_mempolicy([");
	printxval(policies, (unsigned) *mode, "MPOL_???");
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
