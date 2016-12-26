/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include DEF_MPERS_TYPE(struct_msghdr)

#include "msghdr.h"
typedef struct msghdr struct_msghdr;

#include MPERS_DEFS

/*
 * On success, return the number of fetched bytes.
 * On error, return 0;
 *
 * This function cannot use umove_or_printaddr because
 * it is called by dumpio and therefore cannot print anything.
 */

MPERS_PRINTER_DECL(int, fetch_struct_msghdr,
		   struct tcb *const tcp, const kernel_ulong_t addr,
		   void *const p)
{
	struct msghdr *const p_native = p;
	struct_msghdr v_compat;

	if (sizeof(*p_native) == sizeof(v_compat))
		return umove(tcp, addr, p_native) ? 0 : sizeof(*p_native);

	if (umove(tcp, addr, &v_compat))
		return 0;

	p_native->msg_name = (void *) (unsigned long)
	 v_compat.msg_name;

	p_native->msg_namelen =
	 v_compat.msg_namelen;

	p_native->msg_iov = (void *) (unsigned long)
	 v_compat.msg_iov;

	p_native->msg_iovlen =
	 v_compat.msg_iovlen;

	p_native->msg_control = (void *) (unsigned long)
	 v_compat.msg_control;

	p_native->msg_controllen =
	 v_compat.msg_controllen;

	p_native->msg_flags =
	 v_compat.msg_flags;

	return sizeof(v_compat);
}
