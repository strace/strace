/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "xlat/modetypes.h"

void
print_symbolic_mode_t(const unsigned int mode)
{
	const char *ifmt = "";

	if (mode & S_IFMT)
		ifmt = xlookup(modetypes, mode & S_IFMT);

	if (!ifmt || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_03O(mode);

	if (!ifmt || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_ABBREV
		? tprintf_string : tprintf_comment)("%s%s%s%s%s%#03o",
			ifmt, ifmt[0] ? "|" : "",
			(mode & S_ISUID) ? "S_ISUID|" : "",
			(mode & S_ISGID) ? "S_ISGID|" : "",
			(mode & S_ISVTX) ? "S_ISVTX|" : "",
			mode & ~(S_IFMT|S_ISUID|S_ISGID|S_ISVTX));
}

void
print_numeric_umode_t(const unsigned short mode)
{
	PRINT_VAL_03O(mode);
}

void
print_numeric_ll_umode_t(const unsigned long long mode)
{
	PRINT_VAL_03O(mode);
}
