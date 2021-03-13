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

#include DEF_MPERS_TYPE(shmid_ds_t)
#include DEF_MPERS_TYPE(struct_shm_info_t)
#include DEF_MPERS_TYPE(struct_shm_ipc_info_t)

#include "ipc_defs.h"

#include SHM_H_PROVIDER
typedef struct NAME_OF_STRUCT_SHMID_DS shmid_ds_t;
typedef struct shm_info struct_shm_info_t;
typedef struct NAME_OF_STRUCT_SHMINFO struct_shm_ipc_info_t;

#include MPERS_DEFS

#include "xlat/shmctl_flags.h"

#define key NAME_OF_STRUCT_IPC_PERM_KEY

static void
print_ipc_perm(const typeof_field(shmid_ds_t, shm_perm) *const p,
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
print_shmid_ds(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	shmid_ds_t shmid_ds;

	if (umove_or_printaddr(tcp, addr, &shmid_ds))
		return;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(shmid_ds, shm_perm, print_ipc_perm, cmd);
	if (cmd != IPC_SET) {
		tprint_struct_next();
		PRINT_FIELD_U(shmid_ds, shm_segsz);
		tprint_struct_next();
		PRINT_FIELD_TGID(shmid_ds, shm_cpid, tcp);
		tprint_struct_next();
		PRINT_FIELD_TGID(shmid_ds, shm_lpid, tcp);
		tprint_struct_next();
		PRINT_FIELD_U(shmid_ds, shm_nattch);
		tprint_struct_next();
		PRINT_FIELD_U(shmid_ds, shm_atime);
		tprint_struct_next();
		PRINT_FIELD_U(shmid_ds, shm_dtime);
		tprint_struct_next();
		PRINT_FIELD_U(shmid_ds, shm_ctime);
	}
	tprint_struct_end();
}

static void
print_ipc_info(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	struct_shm_ipc_info_t info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(info, shmmax);
	tprint_struct_next();
	PRINT_FIELD_U(info, shmmin);
	tprint_struct_next();
	PRINT_FIELD_U(info, shmmni);
	tprint_struct_next();
	PRINT_FIELD_U(info, shmseg);
	tprint_struct_next();
	PRINT_FIELD_U(info, shmall);
	tprint_struct_end();
}

static void
print_shm_info(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	struct_shm_info_t info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	tprint_struct_begin();
	PRINT_FIELD_D(info, used_ids);
	tprint_struct_next();
	PRINT_FIELD_U(info, shm_tot);
	tprint_struct_next();
	PRINT_FIELD_U(info, shm_rss);
	tprint_struct_next();
	PRINT_FIELD_U(info, shm_swp);
	tprint_struct_next();
	PRINT_FIELD_U(info, swap_attempts);
	tprint_struct_next();
	PRINT_FIELD_U(info, swap_successes);
	tprint_struct_end();
}

SYS_FUNC(shmctl)
{
	const kernel_ulong_t addr = tcp->u_arg[indirect_ipccall(tcp) ? 3 : 2];
	unsigned int cmd = tcp->u_arg[1];

	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;

	if (entering(tcp)) {
		/* shmid */
		PRINT_VAL_D((int) tcp->u_arg[0]);
		tprint_arg_next();

		/* cmd */
		PRINTCTL(shmctl_flags, tcp->u_arg[1], "SHM_???");
		tprint_arg_next();

		switch (cmd) {
		case IPC_SET:
			/* buf */
			print_shmid_ds(tcp, addr, cmd);
			return RVAL_DECODED;

		case IPC_STAT:
		case SHM_STAT:
		case SHM_STAT_ANY:
		case IPC_INFO:
		case SHM_INFO:
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
		case SHM_STAT:
		case SHM_STAT_ANY:
			/* buf */
			print_shmid_ds(tcp, addr, cmd);
			break;

		case IPC_INFO:
			/* buf */
			print_ipc_info(tcp, addr, cmd);
			break;

		case SHM_INFO:
			/* buf */
			print_shm_info(tcp, addr, cmd);
			break;
		}
	}
	return 0;
}
