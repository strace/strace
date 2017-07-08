/*
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

#include DEF_MPERS_TYPE(struct_sock_fprog)

#include <linux/filter.h>
typedef struct sock_fprog struct_sock_fprog;

#include MPERS_DEFS
#include "bpf_fprog.h"

MPERS_PRINTER_DECL(bool, fetch_bpf_fprog, struct tcb *const tcp,
		   const kernel_ulong_t addr, void *const p)
{
	struct bpf_fprog *pfp = p;
	struct_sock_fprog mfp;

	if ((sizeof(*pfp) == sizeof(mfp))
	    && (offsetof(struct bpf_fprog, filter) ==
		offsetof(struct_sock_fprog, filter)))
		return !umove_or_printaddr(tcp, addr, pfp);

	if (umove_or_printaddr(tcp, addr, &mfp))
		return false;

	pfp->len = mfp.len;
	pfp->filter =
#ifndef IN_MPERS
		(uintptr_t)
#endif
		mfp.filter;
	return true;
}
