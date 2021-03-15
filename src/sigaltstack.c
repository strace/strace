/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(stack_t)

#include <signal.h>

#include MPERS_DEFS

#include "xlat/sigaltstack_flags.h"

static void
print_stack_t(struct tcb *const tcp, const kernel_ulong_t addr)
{
	stack_t ss;

	if (umove_or_printaddr(tcp, addr, &ss))
		return;

	tprint_struct_begin();
	PRINT_FIELD_PTR(ss, ss_sp);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(ss, ss_flags, sigaltstack_flags, "SS_???");
	tprint_struct_next();
	PRINT_FIELD_U(ss, ss_size);
	tprint_struct_end();
}

SYS_FUNC(sigaltstack)
{
	if (entering(tcp)) {
		/* ss */
		print_stack_t(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* old_ss */
		print_stack_t(tcp, tcp->u_arg[1]);
	}
	return 0;
}
