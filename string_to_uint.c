/*
 * Copyright (c) 2001-2018 The strace developers.
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

#include "macros.h"

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

int
string_to_bool(const char *const str, const char *const accepted_ending)
{
#define _(s_) { s_, sizeof(s_) - 1 }
	struct strsz {
		const char *str;
		size_t sz;
	};
	static const struct strsz true_strs[] =
		{ _("y"), _("yes"), _("t"), _("true"), _("1") };
	static const struct strsz false_strs[] =
		{ _("n"), _("no"), _("f"), _("false"), _("0") };

	for (size_t i = 0; i < ARRAY_SIZE(true_strs); i++) {
		if (strncasecmp(str, true_strs[i].str, true_strs[i].sz))
			continue;

		if (!str[true_strs[i].sz] || (accepted_ending &&
		    strchr(accepted_ending, str[true_strs[i].sz])))
			return true;
	}

	for (size_t i = 0; i < ARRAY_SIZE(false_strs); i++) {
		if (strncasecmp(str, false_strs[i].str, false_strs[i].sz))
			continue;

		if (!str[false_strs[i].sz] || (accepted_ending &&
		    strchr(accepted_ending, str[false_strs[i].sz])))
			return false;
	}

	return -1;
}
