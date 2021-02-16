/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define TEST_SYSCALL_INVOKE(sample, pst) \
	syscall(TEST_SYSCALL_NR, sample, pst)
#define PRINT_SYSCALL_HEADER(sample) \
	do { \
		int saved_errno = errno; \
		printf("%s(\"%s\", ", TEST_SYSCALL_STR, sample)
#define PRINT_SYSCALL_FOOTER(rc) \
		errno = saved_errno; \
		printf(") = %s\n", sprintrc(rc)); \
	} while (0)

#include "xstatx.c"
