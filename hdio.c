/*
 * Copyright (c) 2009, 2010 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2011-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_hd_geometry)

#include <linux/hdreg.h>

typedef struct hd_geometry struct_hd_geometry;

#include MPERS_DEFS

MPERS_PRINTER_DECL(int, hdio_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case HDIO_GETGEO:
		if (entering(tcp))
			return 0;
		else {
			struct_hd_geometry geo;

			tprints(", ");
			if (!umove_or_printaddr(tcp, arg, &geo))
				tprintf("{heads=%u, sectors=%u, "
					"cylinders=%hu, start=%lu}",
					(unsigned) geo.heads,
					(unsigned) geo.sectors,
					geo.cylinders,
					(unsigned long) geo.start);
		}
		break;
	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
