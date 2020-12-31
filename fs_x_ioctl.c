/*
 * Copyright (c) 2014 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fs.h>
#include "print_fields.h"

int
fs_x_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
#ifdef FITRIM
	/* First seen in linux-2.6.37 */
	case FITRIM: {
		struct fstrim_range fstrim;

		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &fstrim)) {
			PRINT_FIELD_X("{", fstrim, start);
			PRINT_FIELD_U(", ", fstrim, len);
			PRINT_FIELD_U(", ", fstrim, minlen);
			tprints("}");
		}
		break;
	}
#endif

	/* No arguments */
#ifdef FIFREEZE
	case FIFREEZE:
	case FITHAW:
		break;
#endif

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
