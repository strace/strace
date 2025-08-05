/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ipc_defs.h"

#include MSG_H_PROVIDER

#include "xlat/ipc_msg_flags.h"
#include "xlat/ipc_private.h"
#include "xlat/resource_flags.h"

SYS_FUNC(msgget)
{
	/* key */
	tprints_arg_name("key");
	printxval(ipc_private, (unsigned int) tcp->u_arg[0], NULL);

	/* msgflg */
	tprints_arg_next_name("msgflg");
	tprint_flags_begin();
	if (printflags_in(resource_flags, tcp->u_arg[1] & ~0777, NULL) != 0)
		tprint_flags_or();
	print_numeric_umode_t(tcp->u_arg[1] & 0777);
	tprint_flags_end();
	return RVAL_DECODED;
}

static void
tprint_msgsnd(struct tcb *const tcp, const kernel_ulong_t addr,
	      const kernel_ulong_t count, const unsigned int flags)
{
	/* msgp */
	tprints_arg_next_name("msgp");
	tprint_msgbuf(tcp, addr, count);

	/* msgsz */
	tprints_arg_next_name("msgsz");
	PRINT_VAL_U(count);

	/* msgflg */
	tprints_arg_next_name("msgflg");
	printflags(ipc_msg_flags, flags, "MSG_???");
}

SYS_FUNC(msgsnd)
{
	/* msqid */
	tprints_arg_name("msqid");
	PRINT_VAL_D((int) tcp->u_arg[0]);

	if (indirect_ipccall(tcp)) {
		tprint_msgsnd(tcp, tcp->u_arg[3], tcp->u_arg[1],
			      tcp->u_arg[2]);
	} else {
		tprint_msgsnd(tcp, tcp->u_arg[1], tcp->u_arg[2],
			      tcp->u_arg[3]);
	}
	return RVAL_DECODED;
}

static void
tprint_msgrcv(struct tcb *const tcp, const kernel_ulong_t addr,
	      const kernel_ulong_t count, const kernel_ulong_t msgtyp)
{
	/* msgp */
	tprints_arg_next_name("msgp");
	tprint_msgbuf(tcp, addr, count);

	/* msgsz */
	tprints_arg_next_name("msgsz");
	PRINT_VAL_U(count);

	/* msgtyp */
	tprints_arg_next_name("msgtyp");
	PRINT_VAL_D(truncate_klong_to_current_klongsize(msgtyp));
}

static int
fetch_msgrcv_args(struct tcb *const tcp, const kernel_ulong_t addr,
		  kernel_ulong_t *const pair)
{
	if (current_wordsize == sizeof(*pair)) {
		if (umoven(tcp, addr, 2 * sizeof(*pair), pair))
			return -1;
	} else {
		unsigned int tmp[2];

		if (umove(tcp, addr, &tmp))
			return -1;
		pair[0] = tmp[0];
		pair[1] = (int) tmp[1];
	}
	return 0;
}

SYS_FUNC(msgrcv)
{
	if (entering(tcp)) {
		/* msqid */
		tprints_arg_name("msqid");
		PRINT_VAL_D((int) tcp->u_arg[0]);
	} else {
		if (indirect_ipccall(tcp)) {
			const bool direct =
#ifdef SPARC64
				current_wordsize == 8 ||
#endif
				get_tcb_priv_ulong(tcp) != 0;
			if (direct) {
				tprint_msgrcv(tcp, tcp->u_arg[3],
					      tcp->u_arg[1], tcp->u_arg[4]);
			} else {
				kernel_ulong_t pair[2];

				if (fetch_msgrcv_args(tcp, tcp->u_arg[3], pair)) {
					tprints_arg_next_name("ipc_kludge");
					printaddr(tcp->u_arg[3]);
					tprints_arg_next_name("msgsz");
					PRINT_VAL_U(tcp->u_arg[1]);
				} else {
					tprint_msgrcv(tcp, pair[0],
						      tcp->u_arg[1], pair[1]);
				}
			}
			/* msgflg */
			tprints_arg_next_name("msgflg");
			printflags(ipc_msg_flags, tcp->u_arg[2], "MSG_???");
		} else {
			tprint_msgrcv(tcp, tcp->u_arg[1],
				tcp->u_arg[2], tcp->u_arg[3]);

			/* msgflg */
			tprints_arg_next_name("msgflg");
			printflags(ipc_msg_flags, tcp->u_arg[4], "MSG_???");
		}
	}
	return 0;
}
