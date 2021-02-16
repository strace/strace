/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xlat.h"
#include <stdio.h>

#if !XLAT_RAW
static const char *
lookup_xlat(const struct xlat *xlat, unsigned long long val)
{
	const struct xlat_data *xd = xlat->data;

	for (size_t i = 0; i < xlat->size; i++, xd++) {
		if (!xd->str)
			continue;

		if (xd->val == val) {
			return xd->str;
		}
	}

	return NULL;
}
#endif

int
XLAT_NAME(printxval)(const struct xlat *xlat, unsigned long long val,
		     const char *const dflt)
{
#if XLAT_RAW
	printf("%#llx", val);

	return 1;
#else
	const char *str = lookup_xlat(xlat, val);

# if XLAT_VERBOSE
	printf("%#llx", val);
	if (str || dflt)
		printf(" /* %s */", str ?: dflt);
# else
	if (str) {
		fputs(str, stdout);
	} else {
		printf("%#llx", val);
		if (dflt)
			printf(" /* %s */", dflt);
	}
# endif /* XLAT_VERBOSE */

	return !!str;
#endif /* XLAT_RAW */
}

const char *
XLAT_NAME(sprintxlat)(const char *str, unsigned long long val,
		      const char *const dflt)
{
	static char buf[256];

#if XLAT_RAW
	snprintf(buf, sizeof(buf), "%#llx", val);
#elif XLAT_VERBOSE
	if (str || dflt)
		snprintf(buf, sizeof(buf), "%#llx /* %s */", val, str ?: dflt);
	else
		snprintf(buf, sizeof(buf), "%#llx", val);
#else
	if (str)
		return str;

	if (dflt)
		snprintf(buf, sizeof(buf), "%#llx /* %s */", val, dflt);
	else
		snprintf(buf, sizeof(buf), "%#llx", val);
#endif

	return buf;
}

const char *
XLAT_NAME(sprintxval)(const struct xlat *xlat, unsigned long long val,
		      const char *const dflt)
{
#if XLAT_RAW
	return sprintxlat(NULL, val, dflt);
#else
	return sprintxlat(lookup_xlat(xlat, val), val, dflt);
#endif
}
