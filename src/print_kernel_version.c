/*
 * Kernel version printing routine.
 *
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
print_kernel_version(const unsigned long version)
{
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		tprintf("%#lx", version);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" /* ");

	tprintf("KERNEL_VERSION(%lu, %lu, %lu)",
			version >> 16,
			(version >> 8) & 0xFF,
			version & 0xFF);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" */");
}
