/*
 * Copyright (c) 2001-2018 The strace developers.
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

/**
 * Converts string to a boolean value.  Accepts "y", "yes", "t", "true", and "1"
 * as true, "n", "no", "f", "false", and "0" as false (case-insensitive).
 * str may be terminated with one of the characters provided in accepted_ending,
 * instead of '\0'.
 *
 * @param str             String to convert to boolean value.
 * @param accepted_ending If not NULL, used as a list of characters that can be
 *                        possible string terminators apart from '\0'.
 * @return                true if str has matched against one of true strings,
 *                        false if str has matched against one of false strings,
 *                        -1 if str hasn't been matched.
 */
extern int
string_to_bool(const char *const str, const char *const accepted_ending);

#endif /* !STRACE_STRING_TO_UINT_H */
