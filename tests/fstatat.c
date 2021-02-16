/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifdef HAVE_FSTATAT

# define TEST_SYSCALL_INVOKE(sample, pst) \
	syscall(TEST_SYSCALL_NR, AT_FDCWD, sample, pst, AT_SYMLINK_NOFOLLOW)
# define PRINT_SYSCALL_HEADER(sample) \
	do { \
		int saved_errno = errno; \
		printf("%s(AT_FDCWD, \"%s\", ", TEST_SYSCALL_STR, sample)
# define PRINT_SYSCALL_FOOTER(rc) \
		errno = saved_errno; \
		printf(", AT_SYMLINK_NOFOLLOW) = %s\n", sprintrc(rc)); \
	} while (0)

# include "xstatx.c"

#else

SKIP_MAIN_UNDEFINED("HAVE_FSTATAT")

#endif
