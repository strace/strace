/*
 * Check decoding of umode_t type syscall arguments.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>

#ifndef TEST_SYSCALL_PREFIX_ARGS
# define TEST_SYSCALL_PREFIX_ARGS
#endif
#ifndef TEST_SYSCALL_PREFIX_STR
# define TEST_SYSCALL_PREFIX_STR ""
#endif

static const char sample[] = ".";

static void
test_syscall(unsigned short mode)
{
	unsigned long lmode = (unsigned long) 0xffffffffffff0000ULL | mode;
	long rc = syscall(TEST_SYSCALL_NR, TEST_SYSCALL_PREFIX_ARGS
			  sample, lmode);

	if (mode <= 07)
		printf("%s(%s\"%s\", 00%d) = %s\n",
		       TEST_SYSCALL_STR, TEST_SYSCALL_PREFIX_STR,
		       sample, (int) mode, sprintrc(rc));
	else
		printf("%s(%s\"%s\", %#03ho) = %s\n",
		       TEST_SYSCALL_STR, TEST_SYSCALL_PREFIX_STR,
		       sample, mode, sprintrc(rc));
}

int
main(void)
{
	test_syscall(0);
	test_syscall(0xffff);
	test_syscall(06);
	test_syscall(060);
	test_syscall(0600);
	test_syscall(024);
	test_syscall(S_IFREG);
	test_syscall(S_IFDIR | 06);
	test_syscall(S_IFLNK | 060);
	test_syscall(S_IFIFO | 0600);
	test_syscall(S_IFCHR | 024);
	test_syscall((0xffff & ~S_IFMT) | S_IFBLK);

	puts("+++ exited with 0 +++");
	return 0;
}
