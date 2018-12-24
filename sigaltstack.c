/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015-2018 The strace developers.
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

	tprints("{ss_sp=");
	printaddr(ptr_to_kulong(ss.ss_sp));
	tprints(", ss_flags=");
	printflags(sigaltstack_flags, ss.ss_flags, "SS_???");
	tprintf(", ss_size=%" PRI_klu "}", (kernel_ulong_t) ss.ss_size);
}

SYS_FUNC(sigaltstack)
{
	if (entering(tcp)) {
		print_stack_t(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_stack_t(tcp, tcp->u_arg[1]);
	}
	return 0;
}
