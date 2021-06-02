/*
 * Check decoding of getdents64 syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#define kernel_dirent_type kernel_dirent64_t
#define NR_getdents	__NR_getdents64
#define STR_getdents	"getdents64"

#include "xgetdents.c"

#if VERBOSE
static void
print_dirent(const kernel_dirent_type *d)
{
	const unsigned int d_name_offset = offsetof(kernel_dirent_type, d_name);
	int d_name_len = (int) d->d_reclen - d_name_offset;
	if (d_name_len <= 0)
		error_msg_and_fail("d_name_len = %d", d_name_len);

	printf("{");
	PRINT_FIELD_U(*d, d_ino);
	printf(", ");
	PRINT_FIELD_U(*d, d_off);
	printf(", ");
	PRINT_FIELD_U(*d, d_reclen);
	printf(", d_type=%s, d_name=", str_d_type(d->d_type));
	print_quoted_cstring(d->d_name, d_name_len);
	printf("}");
}
#endif
