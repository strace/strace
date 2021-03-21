/*
 * Copyright (c) 2014 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fs.h>

static void
decode_fstrim_range(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct fstrim_range range;

	if (!umove_or_printaddr(tcp, arg, &range)) {
		tprint_struct_begin();
		PRINT_FIELD_X(range, start);
		tprint_struct_next();
		PRINT_FIELD_U(range, len);
		tprint_struct_next();
		PRINT_FIELD_U(range, minlen);
		tprint_struct_end();
	}
}

#include "xlat/fs_xflags.h"

static void
decode_fsxattr(struct tcb *const tcp, const kernel_ulong_t arg,
	       const bool is_get)
{
	struct fsxattr fsxattr;

	if (!umove_or_printaddr(tcp, arg, &fsxattr)) {
		tprint_struct_begin();
		PRINT_FIELD_FLAGS(fsxattr, fsx_xflags, fs_xflags, "FS_XFLAG_???");
		tprint_struct_next();
		PRINT_FIELD_U(fsxattr, fsx_extsize);
		if (is_get) {
			tprint_struct_next();
			PRINT_FIELD_U(fsxattr, fsx_nextents);
		}
		tprint_struct_next();
		PRINT_FIELD_X(fsxattr, fsx_projid);
		tprint_struct_next();
		PRINT_FIELD_U(fsxattr, fsx_cowextsize);
		tprint_struct_end();
	}
}

int
fs_x_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case FITRIM:
		tprint_arg_next();
		decode_fstrim_range(tcp, arg);
		break;

	case FS_IOC_FSGETXATTR:
		if (entering(tcp))
			return 0;
		tprint_arg_next();
		decode_fsxattr(tcp, arg, true);
		break;

	case FS_IOC_FSSETXATTR:
		tprint_arg_next();
		decode_fsxattr(tcp, arg, false);
		break;

	/* No arguments */
	case FIFREEZE:
	case FITHAW:
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
