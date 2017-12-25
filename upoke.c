/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ptrace.h"
#include "ptrace_pokeuser.c"

int
ptrace_upoke(struct tcb *tcp, unsigned long off, kernel_ulong_t val)
{
	if (ptrace_pokeuser(tcp->pid, off, val) < 0) {
		if (errno != ESRCH)
			perror_msg("upoke: PTRACE_POKEUSER pid:%d @%#lx)",
				   tcp->pid, off);
		return -1;
	}
	return 0;
}
