/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007-2017 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015 Bart Van Assche <bart.vanassche@sandisk.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#ifdef HAVE_SCSI_SG_H

#include DEF_MPERS_TYPE(struct_sg_io_hdr)

# include <scsi/sg.h>

typedef struct sg_io_hdr struct_sg_io_hdr;

#endif /* HAVE_SCSI_SG_H */

#include MPERS_DEFS

#include "xlat/sg_io_info.h"

#ifdef HAVE_SCSI_SG_H
# include "print_fields.h"
# include "xlat/sg_io_dxfer_direction.h"
# include "xlat/sg_io_flags.h"

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

#define PRINT_FIELD_SG_IO_BUFFER(prefix_, where_, field_, size_, count_, tcp_)	\
	do {									\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);			\
		print_sg_io_buffer((tcp_), (mpers_ptr_t)((where_).field_),	\
				   (size_), (count_));				\
	} while (0)

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_sg_io_hdr sg_io;
	static const size_t skip_iid =
		offsetof(struct_sg_io_hdr, dxfer_direction);

	tprints("{interface_id='S', ");
	if (umoven_or_printaddr(tcp, arg + skip_iid, sizeof(sg_io) - skip_iid,
				&sg_io.dxfer_direction)) {
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	PRINT_FIELD_XVAL("", sg_io, dxfer_direction, sg_io_dxfer_direction,
			 "SG_DXFER_???");
	PRINT_FIELD_U(", ", sg_io, cmd_len);
	PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, cmdp, sg_io.cmd_len, 0, tcp);
	PRINT_FIELD_U(", ", sg_io, mx_sb_len);
	PRINT_FIELD_U(", ", sg_io, iovec_count);
	PRINT_FIELD_U(", ", sg_io, dxfer_len);
	PRINT_FIELD_U(", ", sg_io, timeout);
	PRINT_FIELD_FLAGS(", ", sg_io, flags, sg_io_flags, "SG_FLAG_???");

	if (sg_io.dxfer_direction == SG_DXFER_TO_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, dxferp, sg_io.dxfer_len, sg_io.iovec_count, tcp);
	}

	struct_sg_io_hdr *entering_sg_io = malloc(sizeof(*entering_sg_io));
	if (entering_sg_io) {
		memcpy(entering_sg_io, &sg_io, sizeof(sg_io));
		entering_sg_io->interface_id = (unsigned char) 'S';
		set_tcb_priv_data(tcp, entering_sg_io, free);
	}

	return 0;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_sg_io_hdr *entering_sg_io = get_tcb_priv_data(tcp);
	struct_sg_io_hdr sg_io;

	if (umove(tcp, arg, &sg_io) < 0) {
		/* print i/o fields fetched on entering syscall */
		if (entering_sg_io->dxfer_direction == SG_DXFER_FROM_DEV)
			PRINT_FIELD_PTR(", ", *entering_sg_io, dxferp);
		PRINT_FIELD_PTR(", ", *entering_sg_io, sbp);
		return RVAL_IOCTL_DECODED;
	}

	if (sg_io.interface_id != entering_sg_io->interface_id) {
		PRINT_FIELD_U(" => ", sg_io, interface_id);
		return RVAL_IOCTL_DECODED;
	}

	if (sg_io.dxfer_direction == SG_DXFER_FROM_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		uint32_t din_len = sg_io.dxfer_len;
		const char *prefix = NULL;

		if (sg_io.resid > 0 && (unsigned int) sg_io.resid <= din_len)
			din_len -= sg_io.resid;

		if (sg_io.dxfer_direction == SG_DXFER_FROM_DEV)
			prefix = ", ";
		else if (din_len)
			prefix = " => ";

		if (prefix) {
			tprints(prefix);
			PRINT_FIELD_SG_IO_BUFFER("", sg_io, dxferp, din_len,
						 sg_io.iovec_count, tcp);
		}
	}
	PRINT_FIELD_X(", ", sg_io, status);
	PRINT_FIELD_X(", ", sg_io, masked_status);
	PRINT_FIELD_X(", ", sg_io, msg_status);
	PRINT_FIELD_U(", ", sg_io, sb_len_wr);
	PRINT_FIELD_SG_IO_BUFFER(", ", sg_io, sbp, sg_io.sb_len_wr, 0, tcp);
	PRINT_FIELD_X(", ", sg_io, host_status);
	PRINT_FIELD_X(", ", sg_io, driver_status);
	PRINT_FIELD_D(", ", sg_io, resid);
	PRINT_FIELD_U(", ", sg_io, duration);
	PRINT_FIELD_FLAGS(", ", sg_io, info, sg_io_info, "SG_INFO_???");

	return RVAL_IOCTL_DECODED;
}

#else /* !HAVE_SCSI_SG_H */

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints("{interface_id='S', ...}");
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
