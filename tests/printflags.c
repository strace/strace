/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xlat.h"
#include <stdio.h>

int
printflags(const struct xlat *xlat, unsigned long long flags,
	   const char *const dflt)
{
	if (flags == 0 && xlat->data->val == 0 && xlat->data->str) {
		fputs(xlat->data->str, stdout);
		return 1;
	}

	int n;
	size_t i = 0;
	char sep = 0;
	const struct xlat_data *xd = xlat->data;
	for (n = 0; i < xlat->size; xd++, i++) {
		if (!xd->str)
			continue;

		if (xd->val && (flags & xd->val) == xd->val) {
			if (sep)
				putc(sep, stdout);
			else
				sep = '|';
			fputs(xd->str, stdout);
			flags &= ~xd->val;
			n++;
		}
	}

	if (n) {
		if (flags) {
			if (sep)
				putc(sep, stdout);
			printf("%#llx", flags);
			n++;
		}
	} else {
		if (flags) {
			printf("%#llx", flags);
			if (dflt)
				printf(" /* %s */", dflt);
		} else {
			if (dflt)
				putc('0', stdout);
		}
	}

	return n;
}
