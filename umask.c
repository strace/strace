/*
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(umask)
{
	print_numeric_umode_t(tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_OCTAL;
}
