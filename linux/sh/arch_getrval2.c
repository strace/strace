/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

long
getrval2(struct tcb *tcp)
{
	unsigned long val;
	if (upeek(tcp, 4*(REG_REG0+1), &val) < 0)
		return -1;
	return val;
}
