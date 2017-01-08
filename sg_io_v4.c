/*
 * Copyright (c) 2015 Bart Van Assche <bart.vanassche@sandisk.com>
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_LINUX_BSG_H

# include <linux/bsg.h>
# include "xlat/bsg_protocol.h"
# include "xlat/bsg_subprotocol.h"
# include "xlat/bsg_flags.h"

static void
print_sg_io_buffer(struct tcb *const tcp, const kernel_ulong_t addr,
		   const unsigned int len)
{
	printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
}

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_v4 sg_io;
	static const size_t skip_iid = offsetof(struct sg_io_v4, protocol);

	tprints("{guard='Q', ");
	if (umoven_or_printaddr(tcp, arg + skip_iid, sizeof(sg_io) - skip_iid,
				&sg_io.protocol)) {
		tprints("}");
		return RVAL_DECODED | 1;
	}

	tprints("protocol=");
	printxval(bsg_protocol, sg_io.protocol, "BSG_PROTOCOL_???");
	tprints(", subprotocol=");
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
	tprints(", flags=");
	printflags(bsg_flags, sg_io.flags, "BSG_FLAG_???");
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

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_v4 sg_io;
	uint32_t din_len;

	if (umove(tcp, arg, &sg_io) < 0) {
		tprints(", ???");
		return RVAL_DECODED | 1;
	}

	if (sg_io.guard != (unsigned char) 'Q') {
		tprintf(" => guard=%u", sg_io.guard);
		return RVAL_DECODED | 1;
	}

	tprintf(", response[%u]=", sg_io.response_len);
	print_sg_io_buffer(tcp, sg_io.response, sg_io.response_len);
	din_len = sg_io.din_xfer_len;
	if (sg_io.din_resid > 0)
		din_len -= sg_io.din_resid;
	tprintf(", din[%u]=", din_len);
	if (sg_io.din_iovec_count)
		tprint_iov_upto(tcp, sg_io.din_iovec_count, sg_io.din_xferp,
				syserror(tcp) ? IOV_DECODE_ADDR :
				IOV_DECODE_STR, din_len);
	else
		print_sg_io_buffer(tcp, sg_io.din_xferp, din_len);
	tprintf(", driver_status=%u", sg_io.driver_status);
	tprintf(", transport_status=%u", sg_io.transport_status);
	tprintf(", device_status=%u", sg_io.device_status);
	tprintf(", retry_delay=%u", sg_io.retry_delay);
	tprints(", info=");
	printflags(sg_io_info, sg_io.info, "SG_INFO_???");
	tprintf(", duration=%u", sg_io.duration);
	tprintf(", response_len=%u", sg_io.response_len);
	tprintf(", din_resid=%u", sg_io.din_resid);
	tprintf(", dout_resid=%u", sg_io.dout_resid);
	tprintf(", generated_tag=%" PRI__u64, sg_io.generated_tag);
	tprintf(", spare_out=%u", sg_io.spare_out);

	return RVAL_DECODED | 1;
}

#else /* !HAVE_LINUX_BSG_H */

static int
decode_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	tprints("{guard='Q', ...}");
	return RVAL_DECODED | 1;
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
