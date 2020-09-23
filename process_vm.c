/*
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2020 The strace developers.
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
		tprints(", ");
	} else {
		kernel_ulong_t local_iovcnt = tcp->u_arg[2];
		kernel_ulong_t remote_iovcnt = tcp->u_arg[4];
		kernel_ulong_t flags = tcp->u_arg[5];

		/* arg 2: local iov */
		tprint_iov_upto(tcp, local_iovcnt, tcp->u_arg[1],
			   syserror(tcp) ? IOV_DECODE_ADDR : IOV_DECODE_STR,
			   tcp->u_rval);
		/* arg 3: local iovcnt */
		tprintf(", %" PRI_klu ", ", local_iovcnt);
		/* arg 4: remote iov */
		tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3],
			   IOV_DECODE_ADDR);
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %" PRI_klu ", %" PRI_klu, remote_iovcnt, flags);
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
	tprints(", ");
	/* arg 2: local iov */
	tprint_iov(tcp, local_iovcnt, tcp->u_arg[1], IOV_DECODE_STR);
	/* arg 3: local iovcnt */
	tprintf(", %" PRI_klu ", ", local_iovcnt);
	/* arg 4: remote iov */
	tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3], IOV_DECODE_ADDR);
	/* arg 5: remote iovcnt */
	/* arg 6: flags */
	tprintf(", %" PRI_klu ", %" PRI_klu, remote_iovcnt, flags);

	return RVAL_DECODED;
}
