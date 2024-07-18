/*
 * Copyright (c) 2001 Wichert Akkerman <wichert@deephackmode.org>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef MIPS

# include <linux/utsname.h>
# ifdef HAVE_ASM_SYSMIPS_H
#  include <asm/sysmips.h>
# endif

# include "xlat/sysmips_operations.h"

SYS_FUNC(sysmips)
{
	printxval64(sysmips_operations, tcp->u_arg[0], "???");
	tprint_arg_next();

	switch (tcp->u_arg[0]) {
	case MIPS_ATOMIC_SET:
		printaddr(tcp->u_arg[1]);
		tprint_arg_next();

		PRINT_VAL_X(tcp->u_arg[2]);
		return RVAL_DECODED;
	case MIPS_FIXADE:
		PRINT_VAL_X(tcp->u_arg[1]);
		return RVAL_DECODED;
	}

	PRINT_VAL_X(tcp->u_arg[1]);
	tprint_arg_next();

	PRINT_VAL_X(tcp->u_arg[2]);
	tprint_arg_next();

	PRINT_VAL_X(tcp->u_arg[3]);
	return RVAL_DECODED;
}

#endif /* MIPS */
