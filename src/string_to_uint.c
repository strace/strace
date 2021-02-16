/*
 * Copyright (c) 2001-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "string_to_uint.h"

long long
string_to_uint_ex(const char *const str, char **const endptr,
		  const unsigned long long max_val,
		  const char *const accepted_ending)
{
	char *end;
	long long val;

	if (!*str)
		return -1;

	errno = 0;
	val = strtoll(str, &end, 10);

	if (str == end || val < 0 || (unsigned long long) val > max_val
	    || (val == LLONG_MAX && errno == ERANGE))
		return -1;

	if (*end && (!accepted_ending || !strchr(accepted_ending, *end)))
		return -1;

	if (endptr)
		*endptr = end;

	return val;
}
