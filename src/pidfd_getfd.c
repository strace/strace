/*
 * pidfd_getfd() syscall decoder.
 *
 * Copyright (c) 2020-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "number_set.h"

SYS_FUNC(pidfd_getfd)
{
	int pidfd = (int) tcp->u_arg[0];
	int targetfd = (int) tcp->u_arg[1];
	unsigned int flags = (unsigned int) tcp->u_arg[2];

	/* pidfd */
	tprints_arg_name("pidfd");
	printfd(tcp, pidfd);

	/* targetfd */
	tprints_arg_next_name("targetfd");
	pid_t target_pid = pidfd_get_pid(tcp->pid, pidfd);
	if (target_pid > 0)
		printfd_pid(tcp, target_pid, targetfd);
	else
		PRINT_VAL_D(targetfd);

	/* flags */
	tprints_arg_next_name("flags");
	PRINT_VAL_X(flags);

	return RVAL_DECODED | RVAL_FD;
}
