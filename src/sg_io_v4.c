/*
 * Copyright (c) 2015 Bart Van Assche <bart.vanassche@sandisk.com>
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/bsg.h>
#include "xlat/bsg_protocol.h"
#include "xlat/bsg_subprotocol.h"
#include "xlat/bsg_flags.h"

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

#define PRINT_FIELD_SG_IO_BUFFER(where_, field_, size_, count_, tcp_)		\
	do {									\
		tprints_field_name(#field_);					\
		print_sg_io_buffer((tcp_), (where_).field_, (size_), (count_));	\
	} while (0)

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_v4 sg_io;
	static const size_t skip_iid = offsetof(struct sg_io_v4, protocol);

	tprint_struct_begin();
	tprints_field_name("guard");
	tprints_string("'Q'");
	tprint_struct_next();
	if (umoven_or_printaddr(tcp, arg + skip_iid, sizeof(sg_io) - skip_iid,
				&sg_io.protocol)) {
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	PRINT_FIELD_XVAL(sg_io, protocol, bsg_protocol, "BSG_PROTOCOL_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL(sg_io, subprotocol, bsg_subprotocol,
			 "BSG_SUB_PROTOCOL_???");
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, request_len);
	tprint_struct_next();
	PRINT_FIELD_SG_IO_BUFFER(sg_io, request, sg_io.request_len, 0, tcp);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, request_tag);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, request_attr);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, request_priority);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, request_extra);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, max_response_len);

	tprint_struct_next();
	PRINT_FIELD_U(sg_io, dout_iovec_count);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, dout_xfer_len);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, din_iovec_count);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, din_xfer_len);
	tprint_struct_next();
	PRINT_FIELD_SG_IO_BUFFER(sg_io, dout_xferp, sg_io.dout_xfer_len,
				 sg_io.dout_iovec_count, tcp);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, timeout);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sg_io, flags, bsg_flags, "BSG_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, usr_ptr);

	sg_io.guard = (unsigned char) 'Q';
	struct sg_io_v4 *entering_sg_io = xobjdup(&sg_io);
	set_tcb_priv_data(tcp, entering_sg_io, free);

	return 0;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_v4 *entering_sg_io = get_tcb_priv_data(tcp);
	struct sg_io_v4 sg_io;
	uint32_t din_len;

	if (umove(tcp, arg, &sg_io) < 0) {
		/* print i/o fields fetched on entering syscall */
		tprint_struct_next();
		PRINT_FIELD_X(*entering_sg_io, response);
		tprint_struct_next();
		PRINT_FIELD_X(*entering_sg_io, din_xferp);
		return RVAL_IOCTL_DECODED;
	}

	if (sg_io.guard != entering_sg_io->guard) {
		tprint_value_changed();
		PRINT_FIELD_U(sg_io, guard);
		return RVAL_IOCTL_DECODED;
	}

	tprint_struct_next();
	PRINT_FIELD_U(sg_io, response_len);
	tprint_struct_next();
	PRINT_FIELD_SG_IO_BUFFER(sg_io, response, sg_io.response_len, 0, tcp);
	din_len = sg_io.din_xfer_len;
	if (sg_io.din_resid > 0 && (unsigned int) sg_io.din_resid <= din_len)
		din_len -= sg_io.din_resid;
	tprint_struct_next();
	PRINT_FIELD_SG_IO_BUFFER(sg_io, din_xferp, din_len,
				 sg_io.din_iovec_count, tcp);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, driver_status);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, transport_status);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, device_status);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, retry_delay);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(sg_io, info, sg_io_info, "SG_INFO_???");
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, duration);
	tprint_struct_next();
	PRINT_FIELD_U(sg_io, response_len);
	tprint_struct_next();
	PRINT_FIELD_D(sg_io, din_resid);
	tprint_struct_next();
	PRINT_FIELD_D(sg_io, dout_resid);
	tprint_struct_next();
	PRINT_FIELD_X(sg_io, generated_tag);

	return RVAL_IOCTL_DECODED;
}

int
decode_sg_io_v4(struct tcb *const tcp, const kernel_ulong_t arg)
{
	return entering(tcp) ? decode_request(tcp, arg)
			     : decode_response(tcp, arg);
}
