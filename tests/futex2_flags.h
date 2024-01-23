/*
 * Check decoding of FUTEX2_* flags.
 *
 * Copyright (c) 2024 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_FUTEX2_FLAGS_H
# define STRACE_TESTS_FUTEX2_FLAGS_H

# include "tests.h"

static const struct {
	unsigned long val;
	const char *raw;
	const char *abbrev;
	const char *verbose;
} futex2_flags[] = { {
		ARG_STR(0),
		"FUTEX2_SIZE_U8",
		"0 /* FUTEX2_SIZE_U8 */"
	}, {
		ARG_STR(0x1),
		"FUTEX2_SIZE_U16",
		"0x1 /* FUTEX2_SIZE_U16 */"
	}, {
		ARG_STR(0x2),
		"FUTEX2_SIZE_U32",
		"0x2 /* FUTEX2_SIZE_U32 */"
	}, {
		ARG_STR(0x3),
		"FUTEX2_SIZE_U64",
		"0x3 /* FUTEX2_SIZE_U64 */"
	}, {
		ARG_STR(0x4),
		"FUTEX2_SIZE_U8|FUTEX2_NUMA",
		"0x4 /* FUTEX2_SIZE_U8|FUTEX2_NUMA */"
	}, {
		ARG_STR(0x82),
		"FUTEX2_SIZE_U32|FUTEX2_PRIVATE",
		"0x82 /* FUTEX2_SIZE_U32|FUTEX2_PRIVATE */"
	}, {
		ARG_STR(0x87),
		"FUTEX2_SIZE_U64|FUTEX2_NUMA|FUTEX2_PRIVATE",
		"0x87 /* FUTEX2_SIZE_U64|FUTEX2_NUMA|FUTEX2_PRIVATE */"
	}, {
		ARG_STR(0xffffff78),
		"FUTEX2_SIZE_U8|0xffffff78",
		"0xffffff78 /* FUTEX2_SIZE_U8|0xffffff78 */"
	}, {
		ARG_STR(0xffffffff),
		"FUTEX2_SIZE_U64|FUTEX2_NUMA|FUTEX2_PRIVATE|0xffffff78",
		"0xffffffff /* FUTEX2_SIZE_U64|FUTEX2_NUMA|FUTEX2_PRIVATE|0xffffff78 */"
	},
};

#endif
