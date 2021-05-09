/*
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(process_vm_readv)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		printpid(tcp, tcp->u_arg[0], PT_TGID);
		tprint_arg_next();
	} else {
		kernel_ulong_t local_iovcnt = tcp->u_arg[2];
		kernel_ulong_t remote_iovcnt = tcp->u_arg[4];
		kernel_ulong_t flags = tcp->u_arg[5];

		/* arg 2: local iov */
		tprint_iov_upto(tcp, local_iovcnt, tcp->u_arg[1], tcp->u_rval,
				syserror(tcp) ? iov_decode_addr
					      : iov_decode_str,
				NULL);
		tprint_arg_next();

		/* arg 3: local iovcnt */
		PRINT_VAL_U(local_iovcnt);
		tprint_arg_next();

		/* arg 4: remote iov */
		tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3], iov_decode_addr);
		tprint_arg_next();

		/* arg 5: remote iovcnt */
		PRINT_VAL_U(remote_iovcnt);
		tprint_arg_next();

		/* arg 6: flags */
		PRINT_VAL_U(flags);
	}
	return 0;
}

SYS_FUNC(process_vm_writev)
{
	kernel_ulong_t local_iovcnt = tcp->u_arg[2];
	kernel_ulong_t remote_iovcnt = tcp->u_arg[4];
	kernel_ulong_t flags = tcp->u_arg[5];

	/* arg 1: pid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* arg 2: local iov */
	tprint_iov(tcp, local_iovcnt, tcp->u_arg[1], iov_decode_str);
	tprint_arg_next();

	/* arg 3: local iovcnt */
	PRINT_VAL_U(local_iovcnt);
	tprint_arg_next();

	/* arg 4: remote iov */
	tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3], iov_decode_addr);
	tprint_arg_next();

	/* arg 5: remote iovcnt */
	PRINT_VAL_U(remote_iovcnt);
	tprint_arg_next();

	/* arg 6: flags */
	PRINT_VAL_U(flags);

	return RVAL_DECODED;
}
