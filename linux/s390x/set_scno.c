/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_set_scno s390_set_scno
#define ARCH_REGSET s390_regset
#include "../s390/set_scno.c"
#undef ARCH_REGSET
#undef arch_set_scno

#define arch_set_scno s390x_set_scno
#define ARCH_REGSET s390x_regset
#include "../s390/set_scno.c"
#undef ARCH_REGSET
#undef arch_set_scno

static int
arch_set_scno(struct tcb *tcp, kernel_ulong_t scno)
{
	if (tcp->currpers == 1)
		return s390_set_scno(tcp, scno);
	else
		return s390x_set_scno(tcp, scno);
}
