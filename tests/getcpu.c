/*
 * Check decoding of getcpu syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#ifdef __NR_getcpu

# include <stdio.h>
# include <unistd.h>

int
main(void)
{
	unsigned *bogus_cpu =
		(unsigned *) tail_alloc(sizeof(*bogus_cpu)) + 1;
	unsigned *bogus_node =
		(unsigned *) tail_alloc(sizeof(*bogus_node)) + 1;
	unsigned *bogus_tcache =
		(unsigned *) tail_alloc(sizeof(*bogus_tcache)) + 1;

	long res;
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned, cpu);
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned, node);
	long *tcache = tail_alloc(128);

	res = syscall(__NR_getcpu, NULL, NULL, NULL);
	printf("getcpu(NULL, NULL, NULL) = %s\n", sprintrc(res));

	res = syscall(__NR_getcpu, bogus_cpu, bogus_node, bogus_tcache);
	printf("getcpu(%p, %p, %p) = %s\n",
	       bogus_cpu, bogus_node, bogus_tcache, sprintrc(res));

	res = syscall(__NR_getcpu, cpu, node, tcache);
	if (res != 0)
		perror_msg_and_skip("getcpu");

	printf("getcpu([%u], [%u], %p) = 0\n", *cpu, *node, tcache);

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_getcpu");

#endif
