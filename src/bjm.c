/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include "xlat/delete_module_flags.h"

SYS_FUNC(delete_module)
{
	/* name */
	printstr(tcp, tcp->u_arg[0]);

	/* flags */
	tprint_arg_next();
	printflags(delete_module_flags, tcp->u_arg[1], "O_???");

	return RVAL_DECODED;
}

SYS_FUNC(init_module)
{
	/* module_image */
	printaddr(tcp->u_arg[0]);

	/* len */
	tprint_arg_next();
	PRINT_VAL_U(tcp->u_arg[1]);

	/* param_values */
	tprint_arg_next();
	printstr(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}

#include "xlat/module_init_flags.h"

SYS_FUNC(finit_module)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);

	/* param_values */
	tprint_arg_next();
	printstr(tcp, tcp->u_arg[1]);

	/* flags */
	tprint_arg_next();
	printflags(module_init_flags, tcp->u_arg[2], "MODULE_INIT_???");

	return RVAL_DECODED;
}
