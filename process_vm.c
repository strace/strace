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
		tprintf("%ld, ", tcp->u_arg[0]);
	} else {
		/* arg 2: local iov */
		if (syserror(tcp)) {
			printaddr(tcp->u_arg[1]);
		} else {
			tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
		}
		/* arg 3: local iovcnt */
		tprintf(", %lu, ", tcp->u_arg[2]);
		/* arg 4: remote iov */
		if (syserror(tcp)) {
			printaddr(tcp->u_arg[3]);
		} else {
			tprint_iov(tcp, tcp->u_arg[4], tcp->u_arg[3], 0);
		}
		/* arg 5: remote iovcnt */
		/* arg 6: flags */
		tprintf(", %lu, %lu", tcp->u_arg[4], tcp->u_arg[5]);
	}
	return 0;
}

SYS_FUNC(process_vm_writev)
{
	/* arg 1: pid */
	tprintf("%ld, ", tcp->u_arg[0]);
	/* arg 2: local iov */
	tprint_iov(tcp, tcp->u_arg[2], tcp->u_arg[1], 1);
	/* arg 3: local iovcnt */
	tprintf(", %lu, ", tcp->u_arg[2]);
	/* arg 4: remote iov */
	tprint_iov(tcp, tcp->u_arg[4], tcp->u_arg[3], 0);
	/* arg 5: remote iovcnt */
	/* arg 6: flags */
	tprintf(", %lu, %lu", tcp->u_arg[4], tcp->u_arg[5]);

	return RVAL_DECODED;
}
