/*
 * Kernel version printing routine.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
print_kernel_version(const unsigned long version)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		PRINT_VAL_X(version);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_begin();

	const unsigned long ver_major = version >> 16;
	const unsigned long ver_minor = (version >> 8) & 0xFF;
	const unsigned long ver_patch = version & 0xFF;

	tprints_arg_begin("KERNEL_VERSION");

	PRINT_VAL_U(ver_major);
	tprint_arg_next();

	PRINT_VAL_U(ver_minor);
	tprint_arg_next();

	PRINT_VAL_U(ver_patch);
	tprint_arg_end();

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
}
