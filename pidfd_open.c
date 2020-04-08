/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(pidfd_open)
{
	/* pid_t pid */
	tprintf("%d", (int) tcp->u_arg[0]);

	/* unsigned int flags */
	tprintf(", %#x", (unsigned int) tcp->u_arg[1]);

	return RVAL_DECODED | RVAL_FD;
}
