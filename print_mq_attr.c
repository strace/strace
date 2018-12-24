/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
