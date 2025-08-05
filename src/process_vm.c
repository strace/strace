/*
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(process_vm_readv)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		tprints_arg_name("pid");
		printpid(tcp, tcp->u_arg[0], PT_TGID);
	} else {
		kernel_ulong_t local_iovcnt = tcp->u_arg[2];
		kernel_ulong_t remote_iovcnt = tcp->u_arg[4];
		kernel_ulong_t flags = tcp->u_arg[5];

		/* arg 2: local iov */
		tprints_arg_next_name("local_iov");
		tprint_iov_upto(tcp, local_iovcnt, tcp->u_arg[1], tcp->u_rval,
				syserror(tcp) ? iov_decode_addr
					      : iov_decode_str,
				NULL);

		/* arg 3: local iovcnt */
		tprints_arg_next_name("liovcnt");
		PRINT_VAL_U(local_iovcnt);

		/* arg 4: remote iov */
		tprints_arg_next_name("remote_iov");
		tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3], iov_decode_addr);

		/* arg 5: remote iovcnt */
		tprints_arg_next_name("riovcnt");
		PRINT_VAL_U(remote_iovcnt);

		/* arg 6: flags */
		tprints_arg_next_name("flags");
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
	tprints_arg_name("pid");
	printpid(tcp, tcp->u_arg[0], PT_TGID);

	/* arg 2: local iov */
	tprints_arg_next_name("local_iov");
	tprint_iov(tcp, local_iovcnt, tcp->u_arg[1], iov_decode_str);

	/* arg 3: local iovcnt */
	tprints_arg_next_name("liovcnt");
	PRINT_VAL_U(local_iovcnt);

	/* arg 4: remote iov */
	tprints_arg_next_name("remote_iov");
	tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3], iov_decode_addr);

	/* arg 5: remote iovcnt */
	tprints_arg_next_name("riovcnt");
	PRINT_VAL_U(remote_iovcnt);

	/* arg 6: flags */
	tprints_arg_next_name("flags");
	PRINT_VAL_U(flags);

	return RVAL_DECODED;
}
