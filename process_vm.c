/*
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
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

SYS_FUNC(process_vm_readv)
{
	if (entering(tcp)) {
		/* arg 1: pid */
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		kernel_ulong_t local_iovcnt = tcp->u_arg[2];
		kernel_ulong_t remote_iovcnt = tcp->u_arg[4];
		kernel_ulong_t flags = tcp->u_arg[5];

		/* arg 2: local iov */
		tprint_iov_upto(tcp, local_iovcnt, tcp->u_arg[1],
			   syserror(tcp) ? IOV_DECODE_ADDR : IOV_DECODE_STR,
			   tcp->u_rval);
		/* arg 3: local iovcnt */
		tprintf(", %" PRI_klu ", ", local_iovcnt);
		/* arg 4: remote iov */
		tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3],
			   IOV_DECODE_ADDR);
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %" PRI_klu ", %" PRI_klu, remote_iovcnt, flags);
	}
	return 0;
}

SYS_FUNC(process_vm_writev)
{
	kernel_ulong_t local_iovcnt = tcp->u_arg[2];
	kernel_ulong_t remote_iovcnt = tcp->u_arg[4];
	kernel_ulong_t flags = tcp->u_arg[5];

	/* arg 1: pid */
	tprintf("%d, ", (int) tcp->u_arg[0]);
	/* arg 2: local iov */
	tprint_iov(tcp, local_iovcnt, tcp->u_arg[1], IOV_DECODE_STR);
	/* arg 3: local iovcnt */
	tprintf(", %" PRI_klu ", ", local_iovcnt);
	/* arg 4: remote iov */
	tprint_iov(tcp, remote_iovcnt, tcp->u_arg[3], IOV_DECODE_ADDR);
	/* arg 5: remote iovcnt */
	/* arg 6: flags */
	tprintf(", %" PRI_klu ", %" PRI_klu, remote_iovcnt, flags);

	return RVAL_DECODED;
}
