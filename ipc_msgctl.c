/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(msqid_ds_t)

#include "ipc_defs.h"

#include MSG_H_PROVIDER
typedef struct NAME_OF_STRUCT_MSQID_DS msqid_ds_t;

#include MPERS_DEFS

#include "print_fields.h"
#include "xlat/msgctl_flags.h"

#define key NAME_OF_STRUCT_IPC_PERM_KEY

static void
print_msqid_ds(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	msqid_ds_t msqid_ds;

	if (umove_or_printaddr(tcp, addr, &msqid_ds))
		return;

	PRINT_FIELD_UID("{msg_perm={", msqid_ds.msg_perm, uid);
	PRINT_FIELD_UID(", ", msqid_ds.msg_perm, gid);
	PRINT_FIELD_NUMERIC_UMODE_T(", ", msqid_ds.msg_perm, mode);
	if (cmd != IPC_SET) {
		PRINT_FIELD_U(", ", msqid_ds.msg_perm, key);
		PRINT_FIELD_UID(", ", msqid_ds.msg_perm, cuid);
		PRINT_FIELD_UID(", ", msqid_ds.msg_perm, cgid);
	}
	tprints("}");
	if (cmd != IPC_SET) {
		PRINT_FIELD_U(", ", msqid_ds, msg_stime);
		PRINT_FIELD_U(", ", msqid_ds, msg_rtime);
		PRINT_FIELD_U(", ", msqid_ds, msg_ctime);
		PRINT_FIELD_U(", ", msqid_ds, msg_qnum);
	}
	PRINT_FIELD_U(", ", msqid_ds, msg_qbytes);
	if (cmd != IPC_SET) {
		PRINT_FIELD_D(", ", msqid_ds, msg_lspid);
		PRINT_FIELD_D(", ", msqid_ds, msg_lrpid);
	}
	tprints("}");
}

static void
print_msginfo(struct tcb *const tcp, const kernel_ulong_t addr,
	      const unsigned int cmd)
{
	struct msginfo info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	PRINT_FIELD_D("{", info, msgpool);
	PRINT_FIELD_D(", ", info, msgmap);
	PRINT_FIELD_D(", ", info, msgmax);
	PRINT_FIELD_D(", ", info, msgmnb);
	PRINT_FIELD_D(", ", info, msgmni);
	PRINT_FIELD_D(", ", info, msgssz);
	PRINT_FIELD_D(", ", info, msgtql);
	PRINT_FIELD_U(", ", info, msgseg);
	tprints("}");
}

SYS_FUNC(msgctl)
{
	const kernel_ulong_t addr = tcp->u_arg[indirect_ipccall(tcp) ? 3 : 2];
	unsigned int cmd = tcp->u_arg[1];

	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;

	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		PRINTCTL(msgctl_flags, tcp->u_arg[1], "MSG_???");
		tprints(", ");
		switch (cmd) {
		case IPC_SET:
			print_msqid_ds(tcp, addr, cmd);
			return RVAL_DECODED;

		case IPC_STAT:
		case MSG_STAT:
		case MSG_STAT_ANY:
		case IPC_INFO:
		case MSG_INFO:
			/* decode on exiting */
			break;

		default:
			printaddr(addr);
			return RVAL_DECODED;
		}
	} else {
		switch (cmd) {
		case IPC_STAT:
		case MSG_STAT:
		case MSG_STAT_ANY:
			print_msqid_ds(tcp, addr, cmd);
			break;

		case IPC_INFO:
		case MSG_INFO:
			print_msginfo(tcp, addr, cmd);
			break;
		}
	}
	return 0;
}
