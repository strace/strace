/*
 * Check decoding of fsopen syscall.
 *
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_fsopen

# include <stdio.h>
# include <stdint.h>
# include <unistd.h>

static const char *errstr;

static long
k_fsopen(const void *name, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = (uintptr_t) name;
	const kernel_ulong_t arg2 = fill | flags;
	const long rc = syscall(__NR_fsopen, arg1, arg2, bad, bad, bad, bad);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	char *const name1 = tail_alloc(DEFAULT_STRLEN + 2);
	char *const name = name1 + 1;
	const void *const efault = name + DEFAULT_STRLEN + 1;
	const char *const empty = efault - 1;
	fill_memory_ex(name1, DEFAULT_STRLEN + 1, '0', 10);
	name1[DEFAULT_STRLEN + 1] = '\0';

	k_fsopen(name, 0);
	printf("fsopen(\"%s\", 0) = %s\n", name, errstr);

	k_fsopen(name1, 1);
	printf("fsopen(\"%.*s\"..., FSOPEN_CLOEXEC) = %s\n",
	       DEFAULT_STRLEN, name1, errstr);

	k_fsopen(0, 2);
	printf("fsopen(NULL, 0x2 /* FSOPEN_??? */) = %s\n", errstr);

	k_fsopen(efault, 0xfffffffe);
	printf("fsopen(%p, 0xfffffffe /* FSOPEN_??? */) = %s\n", efault, errstr);

	k_fsopen(empty, -1);
	printf("fsopen(\"\", FSOPEN_CLOEXEC|0xfffffffe) = %s\n", errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_fsopen")

#endif
