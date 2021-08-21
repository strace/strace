/*
 * Copyright (c) 2009, 2010 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2011-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_hd_geometry)

#include <linux/hdreg.h>

typedef struct hd_geometry struct_hd_geometry;

#include MPERS_DEFS

#include "xlat/hdio_drive_cmds.h"

#include "gen/generated.h"

static int
print_hdio_getgeo(struct tcb *const tcp, const kernel_ulong_t arg)
{
	if (entering(tcp)) {
		tprint_arg_next();

		return 0;
	}

	/* exiting */
	struct_hd_geometry geo;

	if (umove_or_printaddr(tcp, arg, &geo))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_U(geo, heads);
	tprint_struct_next();
	PRINT_FIELD_U(geo, sectors);
	tprint_struct_next();
	PRINT_FIELD_U(geo, cylinders);
	tprint_struct_next();
	PRINT_FIELD_U(geo, start);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_hdio_drive_cmd(struct tcb *const tcp, const kernel_ulong_t arg)
{
	enum { SECTOR_SIZE = 512 };

	if (entering(tcp)) {
		tprint_arg_next();

		struct hd_drive_cmd_hdr c;
		if (umove_or_printaddr(tcp, arg, &c))
			return RVAL_IOCTL_DECODED;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(c, command, hdio_drive_cmds,
				 "ATA_CMD_???");
		tprint_struct_next();
		PRINT_FIELD_U(c, sector_number);
		tprint_struct_next();
		PRINT_FIELD_U(c, feature);
		tprint_struct_next();
		PRINT_FIELD_U(c, sector_count);
		tprint_struct_end();

		return 0;
	}

	/* exiting */
	struct {
		uint8_t status;
		uint8_t error;
		uint8_t nsector;
		uint8_t sector_count;
	} c;

	if ((syserror(tcp) && tcp->u_error != EIO) || umove(tcp, arg, &c))
		return RVAL_IOCTL_DECODED;

	tprint_value_changed();
	tprint_struct_begin();
	PRINT_FIELD_X(c, status);
	tprint_struct_next();
	PRINT_FIELD_U(c, error);
	tprint_struct_next();
	PRINT_FIELD_U(c, nsector);

	if (c.sector_count) {
		tprint_struct_next();
		tprints_field_name("buf");
		printstr_ex(tcp, arg + 4, c.sector_count * SECTOR_SIZE,
			    QUOTE_FORCE_HEX);
	}

	tprint_struct_end();

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
		if (current_klongsize == sizeof(kernel_ulong_t)) {
			return var_ioctl_HDIO(tcp, code, arg);
		} else {
			/*
			 * HDIO compat has never been supported by the kernel.
			 */
			return RVAL_DECODED;
		}
	}

	return RVAL_IOCTL_DECODED;
}
