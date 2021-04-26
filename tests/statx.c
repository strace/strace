/*
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <linux/stat.h>
#include "xlat.h"
#include "xlat/statx_masks.h"
#include "xlat/statx_attrs.h"
#include "xlat/at_statx_sync_types.h"

#define IS_STATX 1
#define TEST_SYSCALL_STR "statx"
#define STRUCT_STAT struct statx
#define STRUCT_STAT_STR "struct statx"
#define STRUCT_STAT_IS_STAT64 0

static unsigned    TEST_SYSCALL_STATX_FLAGS     = AT_STATX_SYNC_AS_STAT;
static const char *TEST_SYSCALL_STATX_FLAGS_STR = "AT_STATX_SYNC_AS_STAT";
static unsigned    TEST_SYSCALL_STATX_MASK      = STATX_ALL;
static const char *TEST_SYSCALL_STATX_MASK_STR  = "STATX_ALL";

#define TEST_SYSCALL_INVOKE(sample, pst) \
	syscall(__NR_statx, AT_FDCWD, sample, TEST_SYSCALL_STATX_FLAGS, \
		TEST_SYSCALL_STATX_MASK, pst)
#define PRINT_SYSCALL_HEADER(sample) \
	do { \
		int saved_errno = errno; \
		printf("%s(AT_FDCWD, \"%s\", %s, %s, ", \
		       TEST_SYSCALL_STR, sample, TEST_SYSCALL_STATX_FLAGS_STR, \
		       TEST_SYSCALL_STATX_MASK_STR)
#define PRINT_SYSCALL_FOOTER(rc) \
		errno = saved_errno; \
		printf(") = %s\n", sprintrc(rc)); \
	} while (0)

#include "xstatx.c"
