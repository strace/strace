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

#include DEF_MPERS_TYPE(msqid_ds_t)

#include "ipc_defs.h"

#include MSG_H_PROVIDER
typedef struct NAME_OF_STRUCT_MSQID_DS msqid_ds_t;

#include MPERS_DEFS

#include "xlat/msgctl_flags.h"

#define key NAME_OF_STRUCT_IPC_PERM_KEY

static void
print_ipc_perm(const typeof_field(msqid_ds_t, msg_perm) *const p,
	       const unsigned int cmd)
{
	tprint_struct_begin();
	PRINT_FIELD_ID(*p, uid);
	tprint_struct_next();
	PRINT_FIELD_ID(*p, gid);
	tprint_struct_next();
	PRINT_FIELD_OBJ_U(*p, mode, print_numeric_ll_umode_t);
	if (cmd != IPC_SET) {
		tprint_struct_next();
		PRINT_FIELD_U(*p, key);
		tprint_struct_next();
		PRINT_FIELD_ID(*p, cuid);
		tprint_struct_next();
		PRINT_FIELD_ID(*p, cgid);
	}
	tprint_struct_end();
}

static void
print_msqid_ds(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	msqid_ds_t msqid_ds;

	if (umove_or_printaddr(tcp, addr, &msqid_ds))
		return;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(msqid_ds, msg_perm, print_ipc_perm, cmd);
	if (cmd != IPC_SET) {
		tprint_struct_next();
		PRINT_FIELD_U(msqid_ds, msg_stime);
		tprint_struct_next();
		PRINT_FIELD_U(msqid_ds, msg_rtime);
		tprint_struct_next();
		PRINT_FIELD_U(msqid_ds, msg_ctime);
		tprint_struct_next();
		PRINT_FIELD_U(msqid_ds, msg_qnum);
	}
	tprint_struct_next();
	PRINT_FIELD_U(msqid_ds, msg_qbytes);
	if (cmd != IPC_SET) {
		tprint_struct_next();
		PRINT_FIELD_D(msqid_ds, msg_lspid);
		tprint_struct_next();
		PRINT_FIELD_D(msqid_ds, msg_lrpid);
	}
	tprint_struct_end();
}

static void
print_msginfo(struct tcb *const tcp, const kernel_ulong_t addr,
	      const unsigned int cmd)
{
	struct msginfo info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	tprint_struct_begin();
	PRINT_FIELD_D(info, msgpool);
	tprint_struct_next();
	PRINT_FIELD_D(info, msgmap);
	tprint_struct_next();
	PRINT_FIELD_D(info, msgmax);
	tprint_struct_next();
	PRINT_FIELD_D(info, msgmnb);
	tprint_struct_next();
	PRINT_FIELD_D(info, msgmni);
	tprint_struct_next();
	PRINT_FIELD_D(info, msgssz);
	tprint_struct_next();
	PRINT_FIELD_D(info, msgtql);
	tprint_struct_next();
	PRINT_FIELD_U(info, msgseg);
	tprint_struct_end();
}

SYS_FUNC(msgctl)
{
	const kernel_ulong_t addr = tcp->u_arg[indirect_ipccall(tcp) ? 3 : 2];
	unsigned int cmd = tcp->u_arg[1];

	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;

	if (entering(tcp)) {
		/* msqid */
		PRINT_VAL_D((int) tcp->u_arg[0]);
		tprint_arg_next();

		/* cmd */
		PRINTCTL(msgctl_flags, tcp->u_arg[1], "MSG_???");
		tprint_arg_next();

		switch (cmd) {
		case IPC_SET:
			/* buf */
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
			/* buf */
			printaddr(addr);
			return RVAL_DECODED;
		}
	} else {
		switch (cmd) {
		case IPC_STAT:
		case MSG_STAT:
		case MSG_STAT_ANY:
			/* buf */
			print_msqid_ds(tcp, addr, cmd);
			break;

		case IPC_INFO:
		case MSG_INFO:
			/* buf */
			print_msginfo(tcp, addr, cmd);
			break;
		}
	}
	return 0;
}
