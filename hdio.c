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

#include "print_fields.h"

static int
print_hdio_getgeo(struct tcb *const tcp, const kernel_ulong_t arg)
{
	if (entering(tcp)) {
		tprints(", ");

		return 0;
	}

	/* exiting */
	struct_hd_geometry geo;

	if (umove_or_printaddr(tcp, arg, &geo))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_U("{", geo, heads);
	PRINT_FIELD_U(", ", geo, sectors);
	PRINT_FIELD_U(", ", geo, cylinders);
	PRINT_FIELD_U(", ", geo, start);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, hdio_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case HDIO_GETGEO:
		return print_hdio_getgeo(tcp, arg);
	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
