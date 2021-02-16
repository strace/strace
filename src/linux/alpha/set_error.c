/*
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	return upoke(tcp, REG_A3, (alpha_a3 = 1))
	       || upoke(tcp, REG_R0, (alpha_r0 = tcp->u_error));
}

static int
arch_set_success(struct tcb *tcp)
{
	return upoke(tcp, REG_A3, (alpha_a3 = 0))
	       || upoke(tcp, REG_R0, (alpha_r0 = tcp->u_rval));
}
