/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/nbd.h>

#define XLAT_MACROS_ONLY
#include "xlat/nbd_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#include "xlat/nbd_ioctl_flags.h"

int
nbd_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	switch (code) {
	case NBD_DISCONNECT:
	case NBD_CLEAR_SOCK:
	case NBD_DO_IT:
	case NBD_CLEAR_QUE:
	case NBD_PRINT_DEBUG:
		return RVAL_IOCTL_DECODED;

	case NBD_SET_SOCK:
		tprint_arg_next();
		printfd(tcp, arg);
		return RVAL_IOCTL_DECODED;

	case NBD_SET_BLKSIZE:
	case NBD_SET_SIZE:
	case NBD_SET_SIZE_BLOCKS:
	case NBD_SET_TIMEOUT:
		tprint_arg_next();
		PRINT_VAL_U(arg);
		return RVAL_IOCTL_DECODED;

	case NBD_SET_FLAGS:
		tprint_arg_next();
		printflags(nbd_ioctl_flags, arg, "NBD_IOC_FLAG_???");
		return RVAL_IOCTL_DECODED;

	default:
		return RVAL_DECODED;
	}
}
