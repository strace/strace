/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include "xlat/pidfd_open_flags.h"

SYS_FUNC(pidfd_open)
{
	/* pid_t pid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* unsigned int flags */
	printflags(pidfd_open_flags, tcp->u_arg[1], "PIDFD_???");

	return RVAL_DECODED | RVAL_FD;
}
