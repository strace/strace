/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include DEF_MPERS_TYPE(mq_attr_t)

#ifdef HAVE_MQUEUE_H
# include <mqueue.h>
typedef struct mq_attr mq_attr_t;
#elif defined HAVE_LINUX_MQUEUE_H
# include <linux/types.h>
# include <linux/mqueue.h>
typedef struct mq_attr mq_attr_t;
#endif

#include "xlat/mq_attr_flags.h"

#include MPERS_DEFS

MPERS_PRINTER_DECL(void, printmqattr, struct tcb *const tcp,
		   const kernel_ulong_t addr, const bool decode_flags)
{
#if defined HAVE_MQUEUE_H || defined HAVE_LINUX_MQUEUE_H
	mq_attr_t attr;
	if (umove_or_printaddr(tcp, addr, &attr))
		return;
	tprints("{mq_flags=");
	if (decode_flags)
		printflags64(mq_attr_flags,
			     zero_extend_signed_to_ull(attr.mq_flags),
			     "O_???");
	else
		tprintf("%#llx", zero_extend_signed_to_ull(attr.mq_flags));
	tprintf(", mq_maxmsg=%lld, mq_msgsize=%lld, mq_curmsgs=%lld}",
		sign_extend_unsigned_to_ll(attr.mq_maxmsg),
		sign_extend_unsigned_to_ll(attr.mq_msgsize),
		sign_extend_unsigned_to_ll(attr.mq_curmsgs));
#else
	printaddr(addr);
#endif
}
