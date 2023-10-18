/*
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mman.h>
#include "xlat/shadow_stack_flags.h"

SYS_FUNC(map_shadow_stack)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* len */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* flags */
	printflags(shadow_stack_flags, tcp->u_arg[2], "SHADOW_STACK_???");

	return RVAL_DECODED;
}
