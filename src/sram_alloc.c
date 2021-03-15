/*
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef BFIN

# include <bfin_sram.h>

# include "xlat/sram_alloc_flags.h"

SYS_FUNC(sram_alloc)
{
	/* size */
	PRINT_VAL_U(tcp->u_arg[0]);
	tprint_arg_next();

	/* flags */
	printflags64(sram_alloc_flags, tcp->u_arg[1], "???_SRAM");

	return RVAL_DECODED | RVAL_HEX;
}

#endif /* BFIN */
