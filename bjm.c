/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>

#include "xlat/delete_module_flags.h"

SYS_FUNC(delete_module)
{
	printstr(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(delete_module_flags, tcp->u_arg[1], "O_???");

	return RVAL_DECODED;
}

SYS_FUNC(init_module)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	printstr(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}

#include "xlat/module_init_flags.h"

SYS_FUNC(finit_module)
{
	/* file descriptor */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* param_values */
	printstr(tcp, tcp->u_arg[1]);
	tprints(", ");
	/* flags */
	printflags(module_init_flags, tcp->u_arg[2], "MODULE_INIT_???");

	return RVAL_DECODED;
}
