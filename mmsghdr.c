/*
 * Copyright (c) 2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012-2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2010-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include "msghdr.h"

static int
decode_mmsghdr(struct tcb *tcp, const int *const p_user_msg_namelen,
	       const long addr, const bool use_msg_len)
{
	struct mmsghdr mmsg;
	int fetched = fetch_struct_mmsghdr(tcp, addr, &mmsg);

	if (fetched) {
		tprints("{msg_hdr=");
		print_struct_msghdr(tcp, &mmsg.msg_hdr, p_user_msg_namelen,
				    use_msg_len ? mmsg.msg_len : -1UL);
		tprintf(", msg_len=%u}", mmsg.msg_len);
	} else {
		printaddr(addr);
	}

	return fetched;
}

void
decode_mmsgvec(struct tcb *tcp, unsigned long addr, unsigned int len,
	       bool use_msg_len)
{
	if (syserror(tcp)) {
		printaddr(addr);
	} else {
		unsigned int i, fetched;

		tprints("[");
		for (i = 0; i < len; ++i, addr += fetched) {
			if (i)
				tprints(", ");
			fetched = decode_mmsghdr(tcp, 0, addr, use_msg_len);
			if (!fetched)
				break;
		}
		tprints("]");
	}
}

void
dumpiov_in_mmsghdr(struct tcb *tcp, long addr)
{
	unsigned int len = tcp->u_rval;
	unsigned int i, fetched;
	struct mmsghdr mmsg;

	for (i = 0; i < len; ++i, addr += fetched) {
		fetched = fetch_struct_mmsghdr(tcp, addr, &mmsg);
		if (!fetched)
			break;
		tprintf(" = %lu buffers in vector %u\n",
			(unsigned long) mmsg.msg_hdr.msg_iovlen, i);
		dumpiov_upto(tcp, mmsg.msg_hdr.msg_iovlen,
			(long) mmsg.msg_hdr.msg_iov, mmsg.msg_len);
	}
}
