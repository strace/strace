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
		   const unsigned int data_size, const unsigned int iovec_count)
{
	if (iovec_count) {
		tprint_iov_upto(tcp, iovec_count, addr, IOV_DECODE_STR,
				data_size);
	} else {
		printstr_ex(tcp, addr, data_size, QUOTE_FORCE_HEX);
	}
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
	tprintf(", request_len=%u, request=", sg_io.request_len);
	print_sg_io_buffer(tcp, sg_io.request, sg_io.request_len, 0);
	tprintf(", request_tag=%#" PRI__x64, sg_io.request_tag);
	tprintf(", request_attr=%u", sg_io.request_attr);
	tprintf(", request_priority=%u", sg_io.request_priority);
	tprintf(", request_extra=%u", sg_io.request_extra);
	tprintf(", max_response_len=%u", sg_io.max_response_len);

	tprintf(", dout_iovec_count=%u", sg_io.dout_iovec_count);
	tprintf(", dout_xfer_len=%u", sg_io.dout_xfer_len);
	tprintf(", din_iovec_count=%u", sg_io.din_iovec_count);
	tprintf(", din_xfer_len=%u", sg_io.din_xfer_len);
	tprints(", dout_xferp=");
	print_sg_io_buffer(tcp, sg_io.dout_xferp, sg_io.dout_xfer_len,
			   sg_io.dout_iovec_count);

	tprintf(", timeout=%u", sg_io.timeout);
	tprints(", flags=");
	printflags(bsg_flags, sg_io.flags, "BSG_FLAG_???");
	tprintf(", usr_ptr=%#" PRI__x64, sg_io.usr_ptr);

	struct sg_io_v4 *entering_sg_io = malloc(sizeof(*entering_sg_io));
	if (entering_sg_io) {
		memcpy(entering_sg_io, &sg_io, sizeof(sg_io));
		entering_sg_io->guard = (unsigned char) 'Q';
		set_tcb_priv_data(tcp, entering_sg_io, free);
	}

	return 1;
}

static int
decode_response(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct sg_io_v4 *entering_sg_io = get_tcb_priv_data(tcp);
	struct sg_io_v4 sg_io;
	uint32_t din_len;

	if (umove(tcp, arg, &sg_io) < 0) {
		/* print i/o fields fetched on entering syscall */
		tprints(", response=");
		printaddr(entering_sg_io->response);
		tprints(", din_xferp=");
		printaddr(entering_sg_io->din_xferp);
		return RVAL_DECODED | 1;
	}

	if (sg_io.guard != entering_sg_io->guard) {
		tprintf(" => guard=%u", sg_io.guard);
		return RVAL_DECODED | 1;
	}

	tprintf(", response_len=%u, response=", sg_io.response_len);
	print_sg_io_buffer(tcp, sg_io.response, sg_io.response_len, 0);
	din_len = sg_io.din_xfer_len;
	if (sg_io.din_resid > 0 && (unsigned int) sg_io.din_resid <= din_len)
		din_len -= sg_io.din_resid;
	tprints(", din_xferp=");
	print_sg_io_buffer(tcp, sg_io.din_xferp, din_len,
			   sg_io.din_iovec_count);
	tprintf(", driver_status=%#x", sg_io.driver_status);
	tprintf(", transport_status=%#x", sg_io.transport_status);
	tprintf(", device_status=%#x", sg_io.device_status);
	tprintf(", retry_delay=%u", sg_io.retry_delay);
	tprints(", info=");
	printflags(sg_io_info, sg_io.info, "SG_INFO_???");
	tprintf(", duration=%u", sg_io.duration);
	tprintf(", response_len=%u", sg_io.response_len);
	tprintf(", din_resid=%d", sg_io.din_resid);
	tprintf(", dout_resid=%d", sg_io.dout_resid);
	tprintf(", generated_tag=%#" PRI__x64, sg_io.generated_tag);

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
