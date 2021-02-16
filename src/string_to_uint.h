/*
 * Copyright (c) 2001-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_STRING_TO_UINT_H
# define STRACE_STRING_TO_UINT_H

# include <limits.h>

# include "kernel_types.h"

extern long long
string_to_uint_ex(const char *str, char **endptr,
		  unsigned long long max_val, const char *accepted_ending);

static inline long long
string_to_uint_upto(const char *const str, const unsigned long long max_val)
{
	return string_to_uint_ex(str, NULL, max_val, NULL);
}

static inline int
string_to_uint(const char *str)
{
	return string_to_uint_upto(str, INT_MAX);
}

static inline long
string_to_ulong(const char *str)
{
	return string_to_uint_upto(str, LONG_MAX);
}

static inline kernel_long_t
string_to_kulong(const char *str)
{
	return string_to_uint_upto(str, ((kernel_ulong_t) -1ULL) >> 1);
}

static inline long long
string_to_ulonglong(const char *str)
{
	return string_to_uint_upto(str, LLONG_MAX);
}

#endif /* !STRACE_STRING_TO_UINT_H */
