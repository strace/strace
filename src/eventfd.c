/*
 * Copyright (c) 2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2008-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include "xlat/efd_flags.h"

static int
do_eventfd(struct tcb *tcp, int flags_arg)
{
	/* initval */
	unsigned int initval = tcp->u_arg[0];
	PRINT_VAL_U(initval);

	if (flags_arg >= 0) {
		/* flags */
		tprint_arg_next();
		printflags(efd_flags, tcp->u_arg[flags_arg], "EFD_???");
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(eventfd)
{
	return do_eventfd(tcp, -1);
}

SYS_FUNC(eventfd2)
{
	return do_eventfd(tcp, 1);
}
