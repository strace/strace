/*
 * pidfd_getfd() syscall decoder.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(pidfd_getfd)
{
	/* int pidfd */
	printfd(tcp, (int) tcp->u_arg[0]);
	/* int fd */
	tprints(", ");
	print_pid_fd(tcp, (int) tcp->u_arg[0], (int) tcp->u_arg[1]);
	/* unsigned int flags */
	tprintf(", %#x", (unsigned int) tcp->u_arg[2]);

	return RVAL_DECODED | RVAL_FD;
}
