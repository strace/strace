/*
 * Check decoding of getdents syscall.
 *
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_getdents

# define kernel_dirent_type kernel_dirent_t
# define NR_getdents	__NR_getdents
# define STR_getdents	"getdents"

# include "xgetdents.c"

# if VERBOSE
static void
print_dirent(const kernel_dirent_type *d)
{
	const unsigned int d_name_offset = offsetof(kernel_dirent_type, d_name);
	int d_name_len = (int) d->d_reclen - d_name_offset - 1;
	if (d_name_len <= 0)
		error_msg_and_fail("d_name_len = %d", d_name_len);

	printf("{");
	PRINT_FIELD_U(*d, d_ino);
	printf(", ");
	PRINT_FIELD_U(*d, d_off);
	printf(", ");
	PRINT_FIELD_U(*d, d_reclen);
	printf(", d_name=");
	print_quoted_cstring(d->d_name, d_name_len);
	printf(", d_type=%s}",
	       str_d_type(*((const char *) d + d->d_reclen - 1)));
}
# endif

#else

SKIP_MAIN_UNDEFINED("__NR_getdents")

#endif
