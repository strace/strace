/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 1999-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ptrace.h"

int
upeek(struct tcb *tcp, unsigned long off, kernel_ulong_t *res)
{
	long val;

	errno = 0;
	val = ptrace(PTRACE_PEEKUSER, (pid_t) tcp->pid, (void *) off, 0);
	if (val == -1 && errno) {
		if (errno != ESRCH)
			perror_func_msg("PTRACE_PEEKUSER pid:%d @0x%lx)",
					tcp->pid, off);
		return -1;
	}
	*res = (unsigned long) val;
	return 0;
}
