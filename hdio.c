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

#include "xlat/hdio_drive_cmds.h"

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

static int
print_hdio_drive_cmd(struct tcb *const tcp, const kernel_ulong_t arg)
{
	enum { SECTOR_SIZE = 512 };

	struct hd_drive_cmd_hdr c;

	if (entering(tcp)) {
		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		PRINT_FIELD_XVAL("{", c, command, hdio_drive_cmds,
				 "ATA_CMD_???");
		PRINT_FIELD_U(", ", c, sector_number);
		PRINT_FIELD_U(", ", c, feature);
		PRINT_FIELD_U(", ", c, sector_count);
		tprints("}");

		return 0;
	}

	/* exiting */
	if ((syserror(tcp) && tcp->u_error != EIO) || umove(tcp, arg, &c))
		return RVAL_IOCTL_DECODED;

	tprintf(" => {/* status */ %#x, /* error */ %u, /* nsector */ %u",
		c.command, c.sector_number, c.feature);

	if (c.sector_count) {
		tprints(", ");
		printstr_ex(tcp, arg + 4, c.sector_count * SECTOR_SIZE,
			    QUOTE_FORCE_HEX);
	}

	tprints("}");

	return RVAL_IOCTL_DECODED;
}

MPERS_PRINTER_DECL(int, hdio_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case HDIO_GETGEO:
		return print_hdio_getgeo(tcp, arg);
	case HDIO_DRIVE_CMD:
		return print_hdio_drive_cmd(tcp, arg);
	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
