/*
 * Check decoding of prctl PR_SET_VMA operation.
 *
 * Copyright (c) 2019-2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <stdio.h>
#include <unistd.h>
#include <linux/prctl.h>

static const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
static const char *errstr;

static long
pr_set_vma(const kernel_ulong_t op, const void *const addr,
	   const kernel_ulong_t size, const void *const arg)
{
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | PR_SET_VMA;
	const kernel_ulong_t arg2 = op;
	const kernel_ulong_t arg3 = (uintptr_t) addr;
	const kernel_ulong_t arg4 = size;
	const kernel_ulong_t arg5 = (uintptr_t) arg;
	const long rc = syscall(__NR_prctl, arg1, arg2, arg3, arg4, arg5, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	prctl_marker();

	char *const name1 = tail_alloc(DEFAULT_STRLEN + 2);
	char *const name = name1 + 1;
	const void *const efault = name + DEFAULT_STRLEN + 1;
	const char *const empty = efault - 1;
	fill_memory_ex(name1, DEFAULT_STRLEN + 1, '0', 10);
	name1[DEFAULT_STRLEN + 1] = '\0';

	pr_set_vma(0, 0, 1, name);
	printf("prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, NULL, 1, \"%s\")"
	       " = %s\n",
	       name, errstr);

	pr_set_vma(0, empty, 2, name1);
	printf("prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, %p, 2, \"%.*s\"...)"
	       " = %s\n",
	       empty, DEFAULT_STRLEN, name1, errstr);

	pr_set_vma(0, empty, 3, efault);
	printf("prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, %p, 3, %p)"
	       " = %s\n",
	       empty, efault, errstr);

	pr_set_vma(0, empty, 4, 0);
	printf("prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, %p, 4, NULL)"
	       " = %s\n",
	       empty, errstr);

	pr_set_vma(0, efault, 5, empty);
	printf("prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, %p, 5, \"\")"
	       " = %s\n",
	       efault, errstr);

	const kernel_ulong_t bad_op = fill | 0xface1fed;
	const kernel_ulong_t bad_size = fill | 0xface2fed;

	pr_set_vma(bad_op, efault, bad_size, empty);
	printf("prctl(PR_SET_VMA, %#llx /* PR_SET_VMA_??? */, %p, %#llx, %p)"
	       " = %s\n",
	       (unsigned long long) bad_op, efault,
	       (unsigned long long) bad_size, empty, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
