/*
 * Support for decoding of NS_* ioctl commands.
 *
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "nsfs.h"

int
nsfs_ioctl(struct tcb *tcp, unsigned int code, kernel_ulong_t arg)
{
	unsigned int uid;
	switch (code) {
	case NS_GET_USERNS:
	case NS_GET_PARENT:
		return RVAL_IOCTL_DECODED | RVAL_FD;
	case NS_GET_NSTYPE:
		if (entering(tcp))
			return 0;
		if (!syserror(tcp))
			tcp->auxstr = xlookup(setns_types, tcp->u_rval);
		return RVAL_IOCTL_DECODED | RVAL_STR;
	case NS_GET_OWNER_UID:
		if (entering(tcp))
			return 0;
		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &uid)) {
			printuid("[", uid);
			tprints("]");
		}
		return RVAL_IOCTL_DECODED;
	default:
		return RVAL_DECODED;
	}
}
