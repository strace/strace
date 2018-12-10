/*
 * Copyright (c) 2001-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_MACROS_H
#define STRACE_MACROS_H

#include <stdbool.h>
#include <sys/types.h>

#include "gcc_compat.h"

#define ARRAY_SIZE(a_)	(sizeof(a_) / sizeof((a_)[0]) + MUST_BE_ARRAY(a_))

#define ARRSZ_PAIR(a_) a_, ARRAY_SIZE(a_)

#define STRINGIFY(...)		#__VA_ARGS__
#define STRINGIFY_VAL(...)	STRINGIFY(__VA_ARGS__)

#ifndef MAX
# define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
# define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#endif
#define CLAMP(val, min, max)	MIN(MAX(min, val), max)

#ifndef ROUNDUP
# define ROUNDUP(val_, div_) ((((val_) + (div_) - 1) / (div_)) * (div_))
#endif

#ifndef offsetofend
# define offsetofend(type_, member_)	\
	(offsetof(type_, member_) + sizeof(((type_ *)0)->member_))
#endif

static inline bool
is_filled(const char *ptr, char fill, size_t size)
{
	while (size--)
		if (*ptr++ != fill)
			return false;

	return true;
}

#define IS_ARRAY_ZERO(arr_)	\
	is_filled((const char *) (arr_), 0, sizeof(arr_) + MUST_BE_ARRAY(arr_))

#endif /* !STRACE_MACROS_H */
