/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015 Bart Van Assche <bart.vanassche@sandisk.com>
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_SCSI_SG_H

# include DEF_MPERS_TYPE(struct_sg_io_hdr)

# include <scsi/sg.h>

typedef struct sg_io_hdr struct_sg_io_hdr;

#endif /* HAVE_SCSI_SG_H */

#include MPERS_DEFS

#include "xlat/sg_io_info.h"

#ifdef HAVE_SCSI_SG_H
# include "xlat/sg_io_dxfer_direction.h"
# include "xlat/sg_io_flags.h"

static void
print_sg_io_buffer(struct tcb *const tcp, const kernel_ulong_t addr,
		   const unsigned int data_size, const unsigned int iovec_count)
{
	if (iovec_count) {
		tprint_iov_upto(tcp, iovec_count, addr, data_size,
				iov_decode_str, NULL);
	} else {
		printstr_ex(tcp, addr, data_size, QUOTE_FORCE_HEX);
	}
}

# define PRINT_FIELD_SG_IO_BUFFER(where_, field_, size_, count_, tcp_)		\
	do {									\
		tprints_field_name(#field_);					\
		print_sg_io_buffer((tcp_), (mpers_ptr_t)((where_).field_),	\
				   (size_), (count_));				\
	} while (0)

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_sg_io_hdr sg_io;
	static const size_t skip_iid =
		offsetof(struct_sg_io_hdr, dxfer_direction);

	tprint_struct_begin();
	tprints_field_name("interface_id");
	tprints_string("'S'");
	tprint_struct_next();
	if (umoven_or_printaddr(tcp, arg + skip_iid, sizeof(sg_io) - skip_iid,
				&sg_io.dxfer_direction)) {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	PRINT_FIELD_XVAL(sg_io, dxfer_direction, sg_io_dxfer_direction,
			 "SG_DXFER_???");
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, cmd_len);
	tprint_struct_next();
	PRINT_FIELD_SG_IO_BUFFER(sg_io, cmdp, sg_io.cmd_len, 0, tcp);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, mx_sb_len);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, iovec_count);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, dxfer_len);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, timeout);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sg_io, flags, sg_io_flags, "SG_FLAG_???");

	if (sg_io.dxfer_direction == SG_DXFER_TO_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		tprint_struct_next();
		PRINT_FIELD_SG_IO_BUFFER(sg_io, dxferp, sg_io.dxfer_len, sg_io.iovec_count, tcp);
	}

	sg_io.interface_id = (unsigned char) 'S';
	struct_sg_io_hdr *entering_sg_io = xobjdup(&sg_io);
	set_tcb_priv_data(tcp, entering_sg_io, free);

	return 0;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_sg_io_hdr *entering_sg_io = get_tcb_priv_data(tcp);
	struct_sg_io_hdr sg_io;

	if (umove(tcp, arg, &sg_io) < 0) {
		/* print i/o fields fetched on entering syscall */
		if (entering_sg_io->dxfer_direction == SG_DXFER_FROM_DEV) {
			tprint_struct_next();
			PRINT_FIELD_PTR(*entering_sg_io, dxferp);
		}
		tprint_struct_next();
		PRINT_FIELD_PTR(*entering_sg_io, sbp);
		return RVAL_IOCTL_DECODED;
	}

	if (sg_io.interface_id != entering_sg_io->interface_id) {
		tprint_value_changed();
		PRINT_FIELD_U(sg_io, interface_id);
		return RVAL_IOCTL_DECODED;
	}

	if (sg_io.dxfer_direction == SG_DXFER_FROM_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		uint32_t din_len = sg_io.dxfer_len;
		bool print_buffer = false;

		if (sg_io.resid > 0 && (unsigned int) sg_io.resid <= din_len)
			din_len -= sg_io.resid;

		if (sg_io.dxfer_direction == SG_DXFER_FROM_DEV) {
			tprint_struct_next();
			print_buffer = true;
		} else if (din_len) {
			tprint_value_changed();
			print_buffer = true;
		}

		if (print_buffer) {
			PRINT_FIELD_SG_IO_BUFFER(sg_io, dxferp, din_len,
						 sg_io.iovec_count, tcp);
		}
	}
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, status);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, masked_status);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, msg_status);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, sb_len_wr);
	tprint_struct_next();
	PRINT_FIELD_SG_IO_BUFFER(sg_io, sbp, sg_io.sb_len_wr, 0, tcp);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, host_status);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, driver_status);
	tprint_struct_next();
	PRINT_FIELD_D(sg_io, resid);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, duration);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sg_io, info, sg_io_info, "SG_INFO_???");

	return RVAL_IOCTL_DECODED;
}

#else /* !HAVE_SCSI_SG_H */

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprint_struct_begin();
	tprints_field_name("interface_id");
	tprints_string("'S'");
	tprint_struct_next();
	tprint_more_data_follows();
	tprint_struct_end();
	return RVAL_IOCTL_DECODED;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	return 0;
}

#endif

MPERS_PRINTER_DECL(int, decode_sg_io_v3,
		   struct tcb *const tcp, const kernel_ulong_t arg)
{
	return entering(tcp) ? decode_request(tcp, arg)
			     : decode_response(tcp, arg);
}
