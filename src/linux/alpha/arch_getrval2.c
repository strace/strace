/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

long
getrval2(struct tcb *tcp)
{
	unsigned long r20;
	if (upeek(tcp, 20, &r20) < 0)
		return -1;
	return r20;
}
