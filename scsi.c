/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007 Dmitry V. Levin <ldv@altlinux.org>
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

# include <linux/ioctl.h>
# include <scsi/sg.h>

# include "xlat/sg_io_dxfer_direction.h"

# ifdef HAVE_LINUX_BSG_H
#  include <linux/bsg.h>
#  include <sys/uio.h>
#  include "xlat/bsg_protocol.h"
#  include "xlat/bsg_subprotocol.h"
# endif

static bool
print_uchar(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	tprintf("%02x", (unsigned int) (* (unsigned char *) elem_buf));

	return true;
}

static void
print_sg_io_buffer(struct tcb *tcp, const unsigned long addr, const unsigned int len)
{
	unsigned char buf;

	print_array(tcp, addr, len, &buf, sizeof(buf),
		    umoven_or_printaddr, print_uchar, 0);
}

static int
print_sg_io_v3_req(struct tcb *tcp, const long arg)
{
	struct sg_io_hdr sg_io;

	if (umove(tcp, arg, &sg_io) < 0) {
		tprints("???}");
		return RVAL_DECODED | 1;
	}

	printxval(sg_io_dxfer_direction, sg_io.dxfer_direction,
		  "SG_DXFER_???");
	tprintf(", cmd[%u]=", sg_io.cmd_len);
	print_sg_io_buffer(tcp, (unsigned long) sg_io.cmdp, sg_io.cmd_len);
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
					(unsigned long) sg_io.dxferp,
					IOV_DECODE_STR,
					sg_io.dxfer_len);
		else
			print_sg_io_buffer(tcp, (unsigned long) sg_io.dxferp,
					   sg_io.dxfer_len);
	}
	return 1;
}

static void
print_sg_io_v3_res(struct tcb *tcp, const long arg)
{
	struct sg_io_hdr sg_io;

	if (umove(tcp, arg, &sg_io) < 0) {
		tprints(", ???");
		return;
	}

	if (sg_io.dxfer_direction == SG_DXFER_FROM_DEV ||
	    sg_io.dxfer_direction == SG_DXFER_TO_FROM_DEV) {
		uint32_t din_len = sg_io.dxfer_len;

		if (sg_io.resid > 0)
			din_len -= sg_io.resid;
		tprintf(", data[%u]=", din_len);
		if (sg_io.iovec_count)
			tprint_iov_upto(tcp, sg_io.iovec_count,
					(unsigned long) sg_io.dxferp,
					IOV_DECODE_STR,
					din_len);
		else
			print_sg_io_buffer(tcp, (unsigned long) sg_io.dxferp,
					   din_len);
	}
	tprintf(", status=%02x", sg_io.status);
	tprintf(", masked_status=%02x", sg_io.masked_status);
	tprintf(", sb[%u]=", sg_io.sb_len_wr);
	print_sg_io_buffer(tcp, (unsigned long) sg_io.sbp, sg_io.sb_len_wr);
	tprintf(", host_status=%#x", sg_io.host_status);
	tprintf(", driver_status=%#x", sg_io.driver_status);
	tprintf(", resid=%d", sg_io.resid);
	tprintf(", duration=%d", sg_io.duration);
	tprintf(", info=%#x", sg_io.info);
}

#ifdef HAVE_LINUX_BSG_H

static int
print_sg_io_v4_req(struct tcb *tcp, const long arg)
{
	struct sg_io_v4 sg_io;

	if (umove(tcp, arg, &sg_io) < 0) {
		tprints("???}");
		return RVAL_DECODED | 1;
	}

	printxval(bsg_protocol, sg_io.protocol, "BSG_PROTOCOL_???");
	tprints(", ");
	printxval(bsg_subprotocol, sg_io.subprotocol, "BSG_SUB_PROTOCOL_???");
	tprintf(", request[%u]=", sg_io.request_len);
	print_sg_io_buffer(tcp, sg_io.request, sg_io.request_len);
	tprintf(", request_tag=%" PRI__u64, sg_io.request_tag);
	tprintf(", request_attr=%u", sg_io.request_attr);
	tprintf(", request_priority=%u", sg_io.request_priority);
	tprintf(", request_extra=%u", sg_io.request_extra);
	tprintf(", max_response_len=%u", sg_io.max_response_len);

	tprintf(", dout_iovec_count=%u", sg_io.dout_iovec_count);
	tprintf(", dout_xfer_len=%u", sg_io.dout_xfer_len);
	tprintf(", din_iovec_count=%u", sg_io.din_iovec_count);
	tprintf(", din_xfer_len=%u", sg_io.din_xfer_len);
	tprintf(", timeout=%u", sg_io.timeout);
	tprintf(", flags=%u", sg_io.flags);
	tprintf(", usr_ptr=%" PRI__u64, sg_io.usr_ptr);
	tprintf(", spare_in=%u", sg_io.spare_in);
	tprintf(", dout[%u]=", sg_io.dout_xfer_len);
	if (sg_io.dout_iovec_count)
		tprint_iov_upto(tcp, sg_io.dout_iovec_count, sg_io.dout_xferp,
				IOV_DECODE_STR, sg_io.dout_xfer_len);
	else
		print_sg_io_buffer(tcp, sg_io.dout_xferp, sg_io.dout_xfer_len);
	return 1;
}

static void
print_sg_io_v4_res(struct tcb *tcp, const long arg)
{
	struct sg_io_v4 sg_io;
	uint32_t din_len;

	if (umove(tcp, arg, &sg_io) < 0) {
		tprints(", ???");
		return;
	}

	tprintf(", response[%u]=", sg_io.response_len);
	print_sg_io_buffer(tcp, sg_io.response, sg_io.response_len);
	din_len = sg_io.din_xfer_len;
	if (sg_io.din_resid > 0)
		din_len -= sg_io.din_resid;
	tprintf(", din[%u]=", din_len);
	if (sg_io.din_iovec_count)
		tprint_iov_upto(tcp, sg_io.din_iovec_count, sg_io.din_xferp,
				IOV_DECODE_STR, din_len);
	else
		print_sg_io_buffer(tcp, sg_io.din_xferp, din_len);
	tprintf(", driver_status=%u", sg_io.driver_status);
	tprintf(", transport_status=%u", sg_io.transport_status);
	tprintf(", device_status=%u", sg_io.device_status);
	tprintf(", retry_delay=%u", sg_io.retry_delay);
	tprintf(", info=%u", sg_io.info);
	tprintf(", duration=%u", sg_io.duration);
	tprintf(", response_len=%u", sg_io.response_len);
	tprintf(", din_resid=%u", sg_io.din_resid);
	tprintf(", dout_resid=%u", sg_io.dout_resid);
	tprintf(", generated_tag=%" PRI__u64, sg_io.generated_tag);
	tprintf(", spare_out=%u", sg_io.spare_out);
}

#else /* !HAVE_LINUX_BSG_H */

static int
print_sg_io_v4_req(struct tcb *tcp, const long arg)
{
	tprints("...}");
	return RVAL_DECODED | 1;
}

static void
print_sg_io_v4_res(struct tcb *tcp, const long arg)
{
}

#endif

static int
print_sg_io_req(struct tcb *tcp, uint32_t iid, const long arg)
{
	tprintf("{'%c', ", iid);

	switch (iid) {
	case 'S':
		return print_sg_io_v3_req(tcp, arg);
	case 'Q':
		return print_sg_io_v4_req(tcp, arg);
	default:
		tprints("...}");
		return RVAL_DECODED | 1;
	}

}

static void
print_sg_io_res(struct tcb *tcp, uint32_t iid, const long arg)
{
	switch (iid) {
	case 'S':
		print_sg_io_v3_res(tcp, arg);
		break;
	case 'Q':
		print_sg_io_v4_res(tcp, arg);
		break;
	}
}

int
scsi_ioctl(struct tcb *tcp, const unsigned int code, const long arg)
{
	uint32_t iid;

	if (SG_IO != code)
		return RVAL_DECODED;

	if (entering(tcp)) {
		tprints(", ");
		if (!arg || umove(tcp, arg, &iid) < 0) {
			printaddr(arg);
			return RVAL_DECODED | 1;
		} else {
			return print_sg_io_req(tcp, iid, arg);
		}
	} else {
		if (!syserror(tcp)) {
			if (umove(tcp, arg, &iid) < 0)
				tprints(", ???");
			else
				print_sg_io_res(tcp, iid, arg);
		}
		tprints("}");
		return RVAL_DECODED | 1;
	}
}

#endif /* HAVE_SCSI_SG_H */
