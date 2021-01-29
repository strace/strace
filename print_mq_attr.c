/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
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

#include MPERS_DEFS

#include "print_fields.h"
#include "xlat/mq_attr_flags.h"

MPERS_PRINTER_DECL(void, printmqattr, struct tcb *const tcp,
		   const kernel_ulong_t addr, const bool decode_flags)
{
#if defined HAVE_MQUEUE_H || defined HAVE_LINUX_MQUEUE_H
	mq_attr_t attr;
	if (umove_or_printaddr(tcp, addr, &attr))
		return;
	if (decode_flags) {
		tprint_struct_begin();
		PRINT_FIELD_FLAGS(attr, mq_flags, mq_attr_flags, "O_???");
	} else {
		tprint_struct_begin();
		PRINT_FIELD_X(attr, mq_flags);
	}
	PRINT_FIELD_D(", ", attr, mq_maxmsg);
	PRINT_FIELD_D(", ", attr, mq_msgsize);
	PRINT_FIELD_D(", ", attr, mq_curmsgs);
	tprints("}");
#else
	printaddr(addr);
#endif
}
