/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
get_syscall_result_regs(struct tcb *tcp)
{
	/* ABI defines result returned in r9 */
	return upeek(tcp, REG_GENERAL(9), &sh64_r9) < 0 ? -1 : 0;
}
