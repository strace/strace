/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
get_syscall_result_regs(struct tcb *tcp)
{
	return (upeek(tcp, REG_A3, &alpha_a3) < 0 ||
		upeek(tcp, REG_R0, &alpha_r0) < 0) ? -1 : 0;
}
