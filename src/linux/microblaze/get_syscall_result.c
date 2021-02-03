/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
get_syscall_result_regs(struct tcb *tcp)
{
	return upeek(tcp, 3 * 4, &microblaze_r3) < 0 ? -1 : 0;
}
