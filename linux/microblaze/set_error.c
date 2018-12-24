/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static int
arch_set_error(struct tcb *tcp)
{
	microblaze_r3 = -tcp->u_error;
	return upoke(tcp, 3 * 4, microblaze_r3);
}

static int
arch_set_success(struct tcb *tcp)
{
	microblaze_r3 = tcp->u_rval;
	return upoke(tcp, 3 * 4, microblaze_r3);
}
