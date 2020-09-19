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

#include DEF_MPERS_TYPE(shmid_ds_t)
#include DEF_MPERS_TYPE(struct_shm_info_t)
#include DEF_MPERS_TYPE(struct_shm_ipc_info_t)

#include "ipc_defs.h"

#include SHM_H_PROVIDER
typedef struct NAME_OF_STRUCT_SHMID_DS shmid_ds_t;
typedef struct shm_info struct_shm_info_t;
typedef struct NAME_OF_STRUCT_SHMINFO struct_shm_ipc_info_t;

#include MPERS_DEFS

#include "print_fields.h"
#include "xlat/shmctl_flags.h"

#define key NAME_OF_STRUCT_IPC_PERM_KEY

static void
print_shmid_ds(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	shmid_ds_t shmid_ds;

	if (umove_or_printaddr(tcp, addr, &shmid_ds))
		return;

	PRINT_FIELD_UID("{shm_perm={", shmid_ds.shm_perm, uid);
	PRINT_FIELD_UID(", ", shmid_ds.shm_perm, gid);
	PRINT_FIELD_NUMERIC_UMODE_T(", ", shmid_ds.shm_perm, mode);

	if (cmd != IPC_SET) {
		PRINT_FIELD_U(", ", shmid_ds.shm_perm, key);
		PRINT_FIELD_UID(", ", shmid_ds.shm_perm, cuid);
		PRINT_FIELD_UID(", ", shmid_ds.shm_perm, cgid);
	}
	tprints("}");
	if (cmd != IPC_SET) {
		PRINT_FIELD_U(", ", shmid_ds, shm_segsz);
		PRINT_FIELD_TGID(", ", shmid_ds, shm_cpid, tcp);
		PRINT_FIELD_TGID(", ", shmid_ds, shm_lpid, tcp);
		PRINT_FIELD_U(", ", shmid_ds, shm_nattch);
		PRINT_FIELD_U(", ", shmid_ds, shm_atime);
		PRINT_FIELD_U(", ", shmid_ds, shm_dtime);
		PRINT_FIELD_U(", ", shmid_ds, shm_ctime);
	}
	tprints("}");
}

static void
print_ipc_info(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	struct_shm_ipc_info_t info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	PRINT_FIELD_U("{", info, shmmax);
	PRINT_FIELD_U(", ", info, shmmin);
	PRINT_FIELD_U(", ", info, shmmni);
	PRINT_FIELD_U(", ", info, shmseg);
	PRINT_FIELD_U(", ", info, shmall);
	tprints("}");
}

static void
print_shm_info(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd)
{
	struct_shm_info_t info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;

	PRINT_FIELD_D("{", info, used_ids);
	PRINT_FIELD_U(", ", info, shm_tot);
	PRINT_FIELD_U(", ", info, shm_rss);
	PRINT_FIELD_U(", ", info, shm_swp);
	PRINT_FIELD_U(", ", info, swap_attempts);
	PRINT_FIELD_U(", ", info, swap_successes);
	tprints("}");
}

SYS_FUNC(shmctl)
{
	const kernel_ulong_t addr = tcp->u_arg[indirect_ipccall(tcp) ? 3 : 2];
	unsigned int cmd = tcp->u_arg[1];

	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;

	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		PRINTCTL(shmctl_flags, tcp->u_arg[1], "SHM_???");
		tprints(", ");
		switch (cmd) {
		case IPC_SET:
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
			printaddr(addr);
			return RVAL_DECODED;
		}
	} else {
		switch (cmd) {
		case IPC_STAT:
		case SHM_STAT:
		case SHM_STAT_ANY:
			print_shmid_ds(tcp, addr, cmd);
			break;

		case IPC_INFO:
			print_ipc_info(tcp, addr, cmd);
			break;

		case SHM_INFO:
			print_shm_info(tcp, addr, cmd);
			break;
		}
	}
	return 0;
}
