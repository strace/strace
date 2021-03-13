/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ipc_defs.h"

#include MSG_H_PROVIDER

#include DEF_MPERS_TYPE(msgbuf_t)
typedef struct msgbuf msgbuf_t;
#include MPERS_DEFS

MPERS_PRINTER_DECL(void, tprint_msgbuf, struct tcb *const tcp,
		   const kernel_ulong_t addr, const kernel_ulong_t count)
{
	msgbuf_t msg;

	if (!umove_or_printaddr(tcp, addr, &msg)) {
		tprint_struct_begin();
		PRINT_FIELD_D(msg, mtype);
		tprint_struct_next();
		tprints_field_name("mtext");
		printstrn(tcp, addr + sizeof(msg.mtype), count);
		tprint_struct_end();
	}
}
