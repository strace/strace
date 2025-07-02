/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

void
print_syscall_param(const char *name)
{
	if (!Nflag || !name || !*name)
		return;

	tprintf_string("%s: ", name);
}
