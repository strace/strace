/*
 * Copyright (c) 2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2008-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>
#ifdef HAVE_SYS_EVENTFD_H
# include <sys/eventfd.h>
#endif

#include "xlat/efd_flags.h"

static int
do_eventfd(struct tcb *tcp, int flags_arg)
{
	tprintf("%u", (unsigned int) tcp->u_arg[0]);
	if (flags_arg >= 0) {
		tprints(", ");
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
