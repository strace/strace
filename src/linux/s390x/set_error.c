/*
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define arch_set_error s390_set_error
#define arch_set_success s390_set_success
#define ARCH_REGSET s390_regset
#include "../s390/set_error.c"
#undef ARCH_REGSET
#undef arch_set_success
#undef arch_set_error

#define arch_set_error s390x_set_error
#define arch_set_success s390x_set_success
#define ARCH_REGSET s390x_regset
#include "../s390/set_error.c"
#undef ARCH_REGSET
#undef arch_set_success
#undef arch_set_error

static int
arch_set_error(struct tcb *tcp)
{
	if (tcp->currpers == 1)
		return s390_set_error(tcp);
	else
		return s390x_set_error(tcp);
}

static int
arch_set_success(struct tcb *tcp)
{
	if (tcp->currpers == 1)
		return s390_set_success(tcp);
	else
		return s390x_set_success(tcp);
}
