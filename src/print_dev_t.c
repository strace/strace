/*
 * Device number printing routine.
 *
 * Copyright (c) 2016-2021 The strace developers.
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

	tprints_arg_begin("makedev");

	PRINT_VAL_X(dev_major);
	tprint_arg_next();

	PRINT_VAL_X(dev_minor);
	tprint_arg_end();

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
}
