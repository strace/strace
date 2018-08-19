/*
 * Support for decoding of NS_* ioctl commands.
 *
 * Copyright (c) 2017 Nikolay Marchuk <marchuk.nikolay.a@gmail.com>
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
#include "nsfs.h"

int
nsfs_ioctl(struct tcb *tcp, unsigned int code, kernel_ulong_t arg)
{
	unsigned int uid;
	switch (code) {
	case NS_GET_USERNS:
	case NS_GET_PARENT:
		return RVAL_IOCTL_DECODED | RVAL_FD;
	case NS_GET_NSTYPE:
		if (entering(tcp))
			return 0;
		if (!syserror(tcp))
			tcp->auxstr = xlookup(setns_types, tcp->u_rval);
		return RVAL_IOCTL_DECODED | RVAL_STR;
	case NS_GET_OWNER_UID:
		if (entering(tcp))
			return 0;
		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &uid)) {
			printuid("[", uid);
			tprints("]");
		}
		return RVAL_IOCTL_DECODED;
	default:
		return RVAL_DECODED;
	}
}
