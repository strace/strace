/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef ARCH_REGSET
# define ARCH_REGSET s390_regset
#endif

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	tcp->scno = ARCH_REGSET.gprs[2] ?
		    ARCH_REGSET.gprs[2] : ARCH_REGSET.gprs[1];
	return 1;
}
