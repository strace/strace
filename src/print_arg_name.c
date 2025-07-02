/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
tprints_arg_name(const char *name)
{
	if (Nflag)
		tprintf_string("%s=", name);
}

void
tprints_arg_next_name(const char *name)
{
	tprint_arg_next();
	tprints_arg_name(name);
}
