/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ipc_defs.h"

#include SEM_H_PROVIDER

#include "xlat/semop_flags.h"

static bool
print_sembuf(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct sembuf *const sb = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_U(*sb, sem_num);
	tprint_struct_next();
	PRINT_FIELD_D(*sb, sem_op);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*sb, sem_flg, semop_flags, "SEM_???");
	tprint_struct_end();

	return true;
}

static void
tprint_sembuf_array(struct tcb *const tcp, const kernel_ulong_t addr,
		    const unsigned int count)
{
	/* sops */
	struct sembuf sb;
	print_array(tcp, addr, count, &sb, sizeof(sb),
		    tfetch_mem, print_sembuf, 0);
	tprint_arg_next();

	/* nsops */
	PRINT_VAL_U(count);
}

SYS_FUNC(semop)
{
	/* semid */
	PRINT_VAL_D((int) tcp->u_arg[0]);
	tprint_arg_next();

	if (indirect_ipccall(tcp)) {
		tprint_sembuf_array(tcp, tcp->u_arg[3], tcp->u_arg[1]);
	} else {
		tprint_sembuf_array(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return RVAL_DECODED;
}

static int
do_semtimedop(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	/* semid */
	PRINT_VAL_D((int) tcp->u_arg[0]);
	tprint_arg_next();

	if (indirect_ipccall(tcp)) {
		tprint_sembuf_array(tcp, tcp->u_arg[3], tcp->u_arg[1]);
		tprint_arg_next();

		/* timeout */
#if defined(S390) || defined(S390X)
		print_ts(tcp, tcp->u_arg[2]);
#else
		print_ts(tcp, tcp->u_arg[4]);
#endif
	} else {
		tprint_sembuf_array(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprint_arg_next();

		/* timeout */
		print_ts(tcp, tcp->u_arg[3]);
	}
	return RVAL_DECODED;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(semtimedop_time32)
{
	return do_semtimedop(tcp, print_timespec32);
}
#endif

SYS_FUNC(semtimedop_time64)
{
	return do_semtimedop(tcp, print_timespec64);
}

SYS_FUNC(semget)
{
	/* key */
	printxval(ipc_private, (unsigned int) tcp->u_arg[0], NULL);
	tprint_arg_next();

	/* nsems */
	PRINT_VAL_D((int) tcp->u_arg[1]);
	tprint_arg_next();

	/* semflg */
	tprint_flags_begin();
	if (printflags_in(resource_flags, tcp->u_arg[2] & ~0777, NULL) != 0)
		tprint_flags_or();
	print_numeric_umode_t(tcp->u_arg[2] & 0777);
	tprint_flags_end();
	return RVAL_DECODED;
}
