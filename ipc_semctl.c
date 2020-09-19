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

#include DEF_MPERS_TYPE(semid_ds_t)
#include DEF_MPERS_TYPE(semun_ptr_t)

#include "ipc_defs.h"

#include SEM_H_PROVIDER

typedef struct NAME_OF_STRUCT_SEMID_DS semid_ds_t;
typedef kernel_ulong_t semun_ptr_t;

#include MPERS_DEFS

#include "print_fields.h"
#include "xlat/semctl_flags.h"

#define key NAME_OF_STRUCT_IPC_PERM_KEY

static void
print_semid_ds(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd, const bool indirect_addr)
{
	semid_ds_t ds;

	if (!tfetch_mem(tcp, addr, sizeof(ds), &ds)) {
		if (indirect_addr)
			tprints("[");
		printaddr(addr);
		if (indirect_addr)
			tprints("]");
		return;
	}

	PRINT_FIELD_UID("{sem_perm={", ds.sem_perm, uid);
	PRINT_FIELD_UID(", ", ds.sem_perm, gid);
	PRINT_FIELD_NUMERIC_UMODE_T(", ", ds.sem_perm, mode);
	if (cmd != IPC_SET) {
		PRINT_FIELD_U(", ", ds.sem_perm, key);
		PRINT_FIELD_UID(", ", ds.sem_perm, cuid);
		PRINT_FIELD_UID(", ", ds.sem_perm, cgid);
	}
	tprints("}");
	if (cmd != IPC_SET) {
		PRINT_FIELD_U(", ", ds, sem_otime);
		PRINT_FIELD_U(", ", ds, sem_ctime);
		PRINT_FIELD_U(", ", ds, sem_nsems);
	}
	tprints("}");
}

static void
print_seminfo(struct tcb *const tcp, const kernel_ulong_t addr,
	      const unsigned int cmd, const bool indirect_addr)
{
	struct seminfo info;

	if (umove_or_printaddr(tcp, addr, &info))
		return;
	if (!tfetch_mem(tcp, addr, sizeof(info), &info)) {
		if (indirect_addr)
			tprints("[");
		printaddr(addr);
		if (indirect_addr)
			tprints("]");
		return;
	}

	PRINT_FIELD_D("{", info, semmap);
	PRINT_FIELD_D(", ", info, semmni);
	PRINT_FIELD_D(", ", info, semmns);
	PRINT_FIELD_D(", ", info, semmnu);
	PRINT_FIELD_D(", ", info, semmsl);
	PRINT_FIELD_D(", ", info, semopm);
	PRINT_FIELD_D(", ", info, semume);
	PRINT_FIELD_D(", ", info, semusz);
	PRINT_FIELD_D(", ", info, semvmx);
	PRINT_FIELD_D(", ", info, semaem);
	tprints("}");
}

SYS_FUNC(semctl)
{
	kernel_ulong_t addr;
	unsigned int cmd = tcp->u_arg[2];
	const bool indirect_addr = indirect_ipccall(tcp)
#ifdef SPARC64
		    && current_personality != 0
#endif
		    ;

	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;

	if (entering(tcp)) {
		tprintf("%d, %d, ", (int) tcp->u_arg[0], (int) tcp->u_arg[1]);
		PRINTCTL(semctl_flags, tcp->u_arg[2], "SEM_???");
		tprints(", ");

		if (indirect_addr) {
			semun_ptr_t ptr;
			if (umove_or_printaddr(tcp, tcp->u_arg[3], &ptr))
				return RVAL_DECODED;
			addr = ptr;
		} else {
			addr = tcp->u_arg[3];
		}
		switch (cmd) {
		case IPC_SET:
			print_semid_ds(tcp, addr, cmd, indirect_addr);
			return RVAL_DECODED;

		case IPC_STAT:
		case SEM_STAT:
		case SEM_STAT_ANY:
		case IPC_INFO:
		case SEM_INFO:
			/* decode on exiting */
			set_tcb_priv_ulong(tcp, addr);
			break;

		default:
			if (indirect_addr)
				tprints("[");
			printaddr(addr);
			if (indirect_addr)
				tprints("]");
			return RVAL_DECODED;
		}
	} else {
		addr = get_tcb_priv_ulong(tcp);
		switch (cmd) {
		case IPC_STAT:
		case SEM_STAT:
		case SEM_STAT_ANY:
			print_semid_ds(tcp, addr, cmd, indirect_addr);
			break;

		case IPC_INFO:
		case SEM_INFO:
			print_seminfo(tcp, addr, cmd, indirect_addr);
			break;
		}
	}
	return 0;
}
