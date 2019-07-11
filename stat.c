/*
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "stat.h"

static void
decode_struct_stat(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct strace_stat st;

	if (fetch_struct_stat(tcp, addr, &st))
		print_struct_stat(tcp, &st);
}

SYS_FUNC(stat)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		decode_struct_stat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(fstat)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		decode_struct_stat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(newfstatat)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
	} else {
		decode_struct_stat(tcp, tcp->u_arg[2]);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[3], "AT_???");
	}
	return 0;
}
