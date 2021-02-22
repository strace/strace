/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(mq_attr_t)

#include "kernel_types.h"
#include <linux/mqueue.h>
typedef struct mq_attr mq_attr_t;

#include MPERS_DEFS

#include "kernel_fcntl.h"
#include "xlat/mq_attr_flags.h"

MPERS_PRINTER_DECL(void, printmqattr, struct tcb *const tcp,
		   const kernel_ulong_t addr, const bool decode_flags)
{
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
	tprint_struct_next();
	PRINT_FIELD_D(attr, mq_maxmsg);
	tprint_struct_next();
	PRINT_FIELD_D(attr, mq_msgsize);
	tprint_struct_next();
	PRINT_FIELD_D(attr, mq_curmsgs);
	tprint_struct_end();
}
