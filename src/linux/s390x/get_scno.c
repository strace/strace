/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_get_scno s390_get_scno
#define ARCH_REGSET s390_regset
#include "../s390/get_scno.c"
#undef ARCH_REGSET
#undef arch_get_scno

#define arch_get_scno s390x_get_scno
#define ARCH_REGSET s390x_regset
#include "../s390/get_scno.c"
#undef ARCH_REGSET
#undef arch_get_scno

static int
arch_get_scno(struct tcb *tcp)
{
	if (s390x_io.iov_len == sizeof(s390_regset))
		return s390_get_scno(tcp);
	else
		return s390x_get_scno(tcp);
}
