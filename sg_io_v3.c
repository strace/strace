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

# include <scsi/sg.h>
# include "xlat/sg_io_dxfer_direction.h"

static void
print_sg_io_buffer(struct tcb *const tcp, const kernel_ulong_t addr,
		   const unsigned int len)
{
	printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
}

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_hdr sg_io;
	static const size_t skip_iid =
		offsetof(struct sg_io_hdr, dxfer_direction);

	tprints("{interface_id='S', ");
	if (umoven_or_printaddr(tcp, arg + skip_iid, sizeof(sg_io) - skip_iid,
				&sg_io.dxfer_direction)) {
		tprints("}");
		return RVAL_DECODED | 1;
	}

	tprints("dxfer_direction=");
	printxval(sg_io_dxfer_direction, sg_io.dxfer_direction,
		  "SG_DXFER_???");
	tprintf(", cmd[%u]=", sg_io.cmd_len);
	print_sg_io_buffer(tcp, ptr_to_kulong(sg_io.cmdp), sg_io.cmd_len);
	tprintf(", mx_sb_len=%d", sg_io.mx_sb_len);
	tprintf(", iovec_count=%d", sg_io.iovec_count);
	tprintf(", dxfer_len=%u", sg_io.dxfer_len);
	tprintf(", timeout=%u", sg_io.timeout);
	tprintf(", flags=%#x", sg_io.flags);

	if (sg_io.dxfer_direction == SG_DXFER_TO_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		tprintf(", data[%u]=", sg_io.dxfer_len);
		if (sg_io.iovec_count)
			tprint_iov_upto(tcp, sg_io.iovec_count,
					ptr_to_kulong(sg_io.dxferp),
					IOV_DECODE_STR,
					sg_io.dxfer_len);
		else
			print_sg_io_buffer(tcp, ptr_to_kulong(sg_io.dxferp),
					   sg_io.dxfer_len);
	}
	return 1;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_hdr sg_io;

	if (umove(tcp, arg, &sg_io) < 0) {
		tprints(", ???");
		return RVAL_DECODED | 1;
	}

	if (sg_io.interface_id != (unsigned char) 'S') {
		tprintf(" => interface_id=%u", sg_io.interface_id);
		return RVAL_DECODED | 1;
	}

	if (sg_io.dxfer_direction == SG_DXFER_FROM_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		uint32_t din_len = sg_io.dxfer_len;

		if (sg_io.resid > 0)
			din_len -= sg_io.resid;
		tprintf(", data[%u]=", din_len);
		if (sg_io.iovec_count)
			tprint_iov_upto(tcp, sg_io.iovec_count,
					ptr_to_kulong(sg_io.dxferp),
					syserror(tcp) ? IOV_DECODE_ADDR :
					IOV_DECODE_STR, din_len);
		else
			print_sg_io_buffer(tcp, ptr_to_kulong(sg_io.dxferp),
					   din_len);
	}
	tprintf(", status=%02x", sg_io.status);
	tprintf(", masked_status=%02x", sg_io.masked_status);
	tprintf(", sb[%u]=", sg_io.sb_len_wr);
	print_sg_io_buffer(tcp, ptr_to_kulong(sg_io.sbp), sg_io.sb_len_wr);
	tprintf(", host_status=%#x", sg_io.host_status);
	tprintf(", driver_status=%#x", sg_io.driver_status);
	tprintf(", resid=%d", sg_io.resid);
	tprintf(", duration=%d", sg_io.duration);
	tprintf(", info=%#x", sg_io.info);

	return RVAL_DECODED | 1;
}

#else /* !HAVE_SCSI_SG_H */

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints("{interface_id='S', ...}");
	return RVAL_DECODED | 1;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	return 0;
}

#endif

int
decode_sg_io_v3(struct tcb *const tcp, const kernel_ulong_t arg)
{
	return entering(tcp) ? decode_request(tcp, arg)
			     : decode_response(tcp, arg);
}
