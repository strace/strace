/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(lookup_dcookie)
{
	if (entering(tcp))
		return 0;

	/* cookie */
	print_syscall_param("cookie");
	unsigned int argn = print_arg_llu(tcp, 0);
	tprint_arg_next();

	/* buffer */
	print_syscall_param("buffer");
	if (syserror(tcp))
		printaddr(tcp->u_arg[argn]);
	else
		printstrn(tcp, tcp->u_arg[argn], tcp->u_rval);
	tprint_arg_next();

	/* len */
	print_syscall_param("len");
	PRINT_VAL_U(tcp->u_arg[argn + 1]);

	return 0;
}
