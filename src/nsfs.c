/*
 * Support for decoding of NS_* ioctl commands.
 *
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
 * Copyright (c) 2017-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/nsfs.h>

static void
print_mnt_ns_info(struct tcb *tcp, unsigned int usize, kernel_ulong_t addr)
{
	struct mnt_ns_info info;
	CHECK_TYPE_SIZE(info, MNT_NS_INFO_SIZE_VER0);
	unsigned int len = MIN(sizeof(info), usize);

	if (umoven_or_printaddr(tcp, addr, len, &info))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(info, size);

	tprint_struct_next();
	PRINT_FIELD_U(info, nr_mounts);

	tprint_struct_next();
	PRINT_FIELD_X(info, mnt_ns_id);

	tprint_struct_end();
}

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
		tprint_arg_next();
		if (!umove_or_printaddr(tcp, arg, &uid)) {
			tprint_indirect_begin();
			printuid(uid);
			tprint_indirect_end();
		}
		return RVAL_IOCTL_DECODED;
	case NS_GET_MNTNS_ID:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		printnum_int64(tcp, arg, "%#" PRIx64);
		return RVAL_IOCTL_DECODED;
	case NS_GET_PID_FROM_PIDNS:
		tprint_arg_next();
		printpid(tcp, arg, PT_NONE);
		return RVAL_IOCTL_DECODED | RVAL_TID;
	case NS_GET_TGID_FROM_PIDNS:
		tprint_arg_next();
		printpid(tcp, arg, PT_NONE);
		return RVAL_IOCTL_DECODED | RVAL_TGID;
	case NS_GET_PID_IN_PIDNS:
		tprint_arg_next();
		printpid(tcp, arg, PT_TID);
		return RVAL_IOCTL_DECODED;
	case NS_GET_TGID_IN_PIDNS:
		tprint_arg_next();
		printpid(tcp, arg, PT_TGID);
		return RVAL_IOCTL_DECODED;
	}

	unsigned int rval_ioctl_decoded = RVAL_IOCTL_DECODED;
	switch (_IOC_NR(code)) {
	case _IOC_NR(NS_MNT_GET_PREV):
	case _IOC_NR(NS_MNT_GET_NEXT):
		rval_ioctl_decoded |= RVAL_FD;
		ATTRIBUTE_FALLTHROUGH;
	case _IOC_NR(NS_MNT_GET_INFO):
		if (_IOC_SIZE(code) >= MNT_NS_INFO_SIZE_VER0) {
			if (entering(tcp))
				return 0;
			tprint_arg_next();
			print_mnt_ns_info(tcp, _IOC_SIZE(code), arg);
			return rval_ioctl_decoded;
		}
		ATTRIBUTE_FALLTHROUGH;
	default:
		return RVAL_DECODED;
	}
}
