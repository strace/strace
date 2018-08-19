/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xlat.h"
#include <stdio.h>

int
printxval(const struct xlat *xlat, unsigned long long val,
	  const char *const dflt)
{
	const struct xlat_data *xd = xlat->data;

	for (size_t i = 0; i < xlat->size; i++, xd++) {
		if (!xd->str)
			continue;

		if (xd->val == val) {
			fputs(xd->str, stdout);
			return 1;
		}
	}

	printf("%#llx", val);
	if (dflt)
		printf(" /* %s */", dflt);
	return 0;
}
