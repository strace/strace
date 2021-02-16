/*
 * Check decoding of invalid pointer by umovestr.
 *
 * Copyright (c) 2016-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <unistd.h>
#include "scno.h"

int
main(void)
{
	if (F8ILL_KULONG_SUPPORTED) {
		const void *const p = tail_memdup(".", 2);
		long rc = syscall(__NR_chdir, p);
		printf("chdir(\".\") = %s\n", sprintrc(rc));

		const kernel_ulong_t ill = f8ill_ptr_to_kulong(p);
		rc = syscall(__NR_chdir, ill);
		printf("chdir(%#llx) = %s\n",
		       (unsigned long long) ill, sprintrc(rc));

		puts("+++ exited with 0 +++");
		return 0;
	} else {
		return 77;
	}
}
