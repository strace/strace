/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

long
getrval2(struct tcb *tcp)
{
	return sh_regs.regs[1];
}
