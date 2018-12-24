/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	sh_r0 = -tcp->u_error;
	return upoke(tcp, 4 * REG_REG0, sh_r0);
}

static int
arch_set_success(struct tcb *tcp)
{
	sh_r0 = tcp->u_rval;
	return upoke(tcp, 4 * REG_REG0, sh_r0);
}
