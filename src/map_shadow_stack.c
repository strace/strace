/*
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2023-2025 The strace developers.
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
	tprints_arg_name("addr");
	printaddr(tcp->u_arg[0]);

	/* len */
	tprints_arg_next_name("len");
	PRINT_VAL_U(tcp->u_arg[1]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(shadow_stack_flags, tcp->u_arg[2], "SHADOW_STACK_???");

	return RVAL_DECODED;
}
