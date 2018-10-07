/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>

#include "xlat/modeflags.h"
#include "xlat/modetypes.h"

void
print_symbolic_mode_t(const unsigned int mode)
{
	const unsigned int fmt = mode & S_IFMT;
	const char *fmt_str = xlookup(modetypes, fmt);
	const unsigned int flags = mode & modeflags->flags_mask;
	bool raw = (!(fmt && fmt_str) && (!flags || fmt))
		   || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW;

	if (raw || xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		PRINT_VAL_03O(mode);
	if (raw)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();
	if (fmt_str) {
		print_xlat_ex(fmt, fmt_str, XLAT_STYLE_ABBREV|XLAT_STYLE_FMT_O);
		tprint_or();
	}
	if (flags) {
		printflags(modeflags, flags, NULL);
		tprint_or();
	}
	PRINT_VAL_03O(mode & ~flags & ~fmt);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
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
