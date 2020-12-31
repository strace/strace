/*
 * Copyright (c) 2014 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "types/fs_x.h"
#define XLAT_MACROS_ONLY
# include "xlat/fs_x_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

int
fs_x_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case FITRIM: {
		struct_fstrim_range fstrim;

		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &fstrim)) {
			PRINT_FIELD_X("{", fstrim, start);
			PRINT_FIELD_U(", ", fstrim, len);
			PRINT_FIELD_U(", ", fstrim, minlen);
			tprints("}");
		}
		break;
	}

	/* No arguments */
	case FIFREEZE:
	case FITHAW:
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
