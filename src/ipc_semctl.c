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

#include DEF_MPERS_TYPE(semid_ds_t)
#include DEF_MPERS_TYPE(semun_ptr_t)

#include "ipc_defs.h"

#include SEM_H_PROVIDER

typedef struct NAME_OF_STRUCT_SEMID_DS semid_ds_t;
typedef kernel_ulong_t semun_ptr_t;

#include MPERS_DEFS

#include "xlat/semctl_flags.h"

#define key NAME_OF_STRUCT_IPC_PERM_KEY

static void
print_ipc_perm(const typeof_field(semid_ds_t, sem_perm) *const p,
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
print_semid_ds(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int cmd, const bool indirect_addr)
{
	semid_ds_t ds;

	if (!tfetch_mem(tcp, addr, sizeof(ds), &ds)) {
		if (indirect_addr)
			tprint_indirect_begin();
		printaddr(addr);
		if (indirect_addr)
			tprint_indirect_end();
		return;
	}

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(ds, sem_perm, print_ipc_perm, cmd);
	if (cmd != IPC_SET) {
		tprint_struct_next();
		PRINT_FIELD_U(ds, sem_otime);
		tprint_struct_next();
		PRINT_FIELD_U(ds, sem_ctime);
		tprint_struct_next();
		PRINT_FIELD_U(ds, sem_nsems);
	}
	tprint_struct_end();
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
			tprint_indirect_begin();
		printaddr(addr);
		if (indirect_addr)
			tprint_indirect_end();
		return;
	}

	tprint_struct_begin();
	PRINT_FIELD_D(info, semmap);
	tprint_struct_next();
	PRINT_FIELD_D(info, semmni);
	tprint_struct_next();
	PRINT_FIELD_D(info, semmns);
	tprint_struct_next();
	PRINT_FIELD_D(info, semmnu);
	tprint_struct_next();
	PRINT_FIELD_D(info, semmsl);
	tprint_struct_next();
	PRINT_FIELD_D(info, semopm);
	tprint_struct_next();
	PRINT_FIELD_D(info, semume);
	tprint_struct_next();
	PRINT_FIELD_D(info, semusz);
	tprint_struct_next();
	PRINT_FIELD_D(info, semvmx);
	tprint_struct_next();
	PRINT_FIELD_D(info, semaem);
	tprint_struct_end();
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
		/* semid */
		PRINT_VAL_D((int) tcp->u_arg[0]);
		tprint_arg_next();

		/* semnum */
		PRINT_VAL_D((int) tcp->u_arg[1]);
		tprint_arg_next();

		/* cmd */
		PRINTCTL(semctl_flags, tcp->u_arg[2], "SEM_???");
		tprint_arg_next();

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
				tprint_indirect_begin();
			printaddr(addr);
			if (indirect_addr)
				tprint_indirect_end();
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
