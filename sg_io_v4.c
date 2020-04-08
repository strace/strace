/*
 * Copyright (c) 2015 Bart Van Assche <bart.vanassche@sandisk.com>
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_BSG_H

# include "print_fields.h"
# include <linux/bsg.h>
# include "xlat/bsg_protocol.h"
# include "xlat/bsg_subprotocol.h"
# include "xlat/bsg_flags.h"

static void
print_sg_io_buffer(struct tcb *const tcp, const kernel_ulong_t addr,
		   const unsigned int data_size, const unsigned int iovec_count)
{
	if (iovec_count) {
		tprint_iov_upto(tcp, iovec_count, addr, IOV_DECODE_STR,
				data_size);
	} else {
		printstr_ex(tcp, addr, data_size, QUOTE_FORCE_HEX);
	}
}

# define PRINT_FIELD_SG_IO_BUFFER(prefix_, where_, field_, size_, count_, tcp_)	\
	do {									\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);			\
		print_sg_io_buffer((tcp_), (where_).field_, (size_), (count_));	\
	} while (0)

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_v4 sg_io;
	static const size_t skip_iid = offsetof(struct sg_io_v4, protocol);

	tprints("{guard='Q', ");
	if (umoven_or_printaddr(tcp, arg + skip_iid, sizeof(sg_io) - skip_iid,
				&sg_io.protocol)) {
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	PRINT_FIELD_XVAL("", sg_io, protocol, bsg_protocol, "BSG_PROTOCOL_???");
	PRINT_FIELD_XVAL(", ", sg_io, subprotocol, bsg_subprotocol,
			 "BSG_SUB_PROTOCOL_???");
	PRINT_FIELD_U(", ", sg_io, request_len);
	PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, request, sg_io.request_len,
				 0, tcp);
	PRINT_FIELD_X(", ", sg_io, request_tag);
	PRINT_FIELD_U(", ", sg_io, request_attr);
	PRINT_FIELD_U(", ", sg_io, request_priority);
	PRINT_FIELD_U(", ", sg_io, request_extra);
	PRINT_FIELD_U(", ", sg_io, max_response_len);

	PRINT_FIELD_U(", ", sg_io, dout_iovec_count);
	PRINT_FIELD_U(", ", sg_io, dout_xfer_len);
	PRINT_FIELD_U(", ", sg_io, din_iovec_count);
	PRINT_FIELD_U(", ", sg_io, din_xfer_len);
	PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, dout_xferp, sg_io.dout_xfer_len,
				 sg_io.dout_iovec_count, tcp);

	PRINT_FIELD_U(", ", sg_io, timeout);
	PRINT_FIELD_FLAGS(", ", sg_io, flags, bsg_flags, "BSG_FLAG_???");
	PRINT_FIELD_X(", ", sg_io, usr_ptr);

	struct sg_io_v4 *entering_sg_io = malloc(sizeof(*entering_sg_io));
	if (entering_sg_io) {
		memcpy(entering_sg_io, &sg_io, sizeof(sg_io));
		entering_sg_io->guard = (unsigned char) 'Q';
		set_tcb_priv_data(tcp, entering_sg_io, free);
	}

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
		PRINT_FIELD_X(", ", *entering_sg_io, response);
		PRINT_FIELD_X(", ", *entering_sg_io, din_xferp);
		return RVAL_IOCTL_DECODED;
	}

	if (sg_io.guard != entering_sg_io->guard) {
		PRINT_FIELD_U(" => ", sg_io, guard);
		return RVAL_IOCTL_DECODED;
	}

	PRINT_FIELD_U(", ", sg_io, response_len);
	PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, response, sg_io.response_len,
				 0, tcp);
	din_len = sg_io.din_xfer_len;
	if (sg_io.din_resid > 0 && (unsigned int) sg_io.din_resid <= din_len)
		din_len -= sg_io.din_resid;
	PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, din_xferp, din_len,
				 sg_io.din_iovec_count, tcp);
	PRINT_FIELD_X(", ", sg_io, driver_status);
	PRINT_FIELD_X(", ", sg_io, transport_status);
	PRINT_FIELD_X(", ", sg_io, device_status);
	PRINT_FIELD_U(", ", sg_io, retry_delay);
	PRINT_FIELD_FLAGS(", ", sg_io, info, sg_io_info, "SG_INFO_???");
	PRINT_FIELD_U(", ", sg_io, duration);
	PRINT_FIELD_U(", ", sg_io, response_len);
	PRINT_FIELD_D(", ", sg_io, din_resid);
	PRINT_FIELD_D(", ", sg_io, dout_resid);
	PRINT_FIELD_X(", ", sg_io, generated_tag);

	return RVAL_IOCTL_DECODED;
}

#else /* !HAVE_LINUX_BSG_H */

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints("{guard='Q', ...}");
	return RVAL_IOCTL_DECODED;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	return 0;
}

#endif

int
decode_sg_io_v4(struct tcb *const tcp, const kernel_ulong_t arg)
{
	return entering(tcp) ? decode_request(tcp, arg)
			     : decode_response(tcp, arg);
}
