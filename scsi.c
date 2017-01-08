/*
 * Copyright (c) 2007 Vladimir Nadvornik <nadvornik@suse.cz>
 * Copyright (c) 2007-2017 Dmitry V. Levin <ldv@altlinux.org>
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
#endif

#ifndef SG_IO
# define SG_IO 0x2285
#endif

static int
decode_sg_io(struct tcb *const tcp, const uint32_t iid,
	     const kernel_ulong_t arg)
{
	switch (iid) {
		case 'S':
			return decode_sg_io_v3(tcp, arg);
		case 'Q':
			return decode_sg_io_v4(tcp, arg);
		default:
			tprintf("[%u]", iid);
			return RVAL_DECODED | 1;
	}

}

int
scsi_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	if (SG_IO != code)
		return RVAL_DECODED;

	if (entering(tcp)) {
		uint32_t iid;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &iid)) {
			return RVAL_DECODED | 1;
		} else {
			return decode_sg_io(tcp, iid, arg);
		}
	} else {
		uint32_t *piid = get_tcb_priv_data(tcp);
		if (piid)
			decode_sg_io(tcp, *piid, arg);
		tprints("}");
		return RVAL_DECODED | 1;
	}
}
