/*
 * Device number printing routine.
 *
 * Copyright (c) 2016-2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sys/sysmacros.h>

void
print_dev_t(const unsigned long long dev)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_X(dev);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	const unsigned int dev_major = major(dev);
	const unsigned int dev_minor = minor(dev);

	tprints_fn_begin("makedev");
	PRINT_VAL_X(dev_major);

	tprint_fn_next();
	PRINT_VAL_X(dev_minor);
	tprint_fn_end();

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
}
