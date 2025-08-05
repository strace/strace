/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>

/* Working around UAPI breakage in v5.19-rc1~89^2~9^2~18 */
#ifndef F_GETLK64
# ifdef __mips__
#  define F_GETLK64	33
#  define F_SETLK64	34
#  define F_SETLKW64	35
# else
#  define F_GETLK64	12
#  define F_SETLK64	13
#  define F_SETLKW64	14
# endif
#endif /* !F_GETLK64 */

#include "xlat/f_owner_types.h"
#include "xlat/f_seals.h"
#include "xlat/fcntlcmds.h"
#include "xlat/fdflags.h"
#include "xlat/lockfcmds.h"
#include "xlat/notifyflags.h"

static void
print_struct_flock64(struct tcb *const tcp, const struct flock64 *fl, const int getlk)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*fl, l_type, lockfcmds, "F_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL(*fl, l_whence, whence_codes, "SEEK_???");
	tprint_struct_next();
	PRINT_FIELD_D(*fl, l_start);
	tprint_struct_next();
	PRINT_FIELD_D(*fl, l_len);
	if (getlk) {
		tprint_struct_next();
		PRINT_FIELD_TGID(*fl, l_pid, tcp);
	}
	tprint_struct_end();
}

static void
printflock64(struct tcb *const tcp, const kernel_ulong_t addr, const int getlk)
{
	struct flock64 fl;

	if (fetch_struct_flock64(tcp, addr, &fl))
		print_struct_flock64(tcp, &fl, getlk);
}

static void
printflock(struct tcb *const tcp, const kernel_ulong_t addr, const int getlk)
{
	struct flock64 fl;

	if (fetch_struct_flock(tcp, addr, &fl))
		print_struct_flock64(tcp, &fl, getlk);
}

static void
print_f_owner_ex(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct { int type, pid; } owner;

	if (umove_or_printaddr(tcp, addr, &owner))
		return;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(owner, type, f_owner_types, "F_OWNER_???");

	enum pid_type pid_type = PT_NONE;
	switch (owner.type)
	{
	case F_OWNER_TID:
		pid_type = PT_TID;
		break;
	case F_OWNER_PID:
		pid_type = PT_TGID;
		break;
	case F_OWNER_PGRP:
		pid_type = PT_PGID;
		break;
	}
	tprint_struct_next();
	tprints_field_name("pid");
	printpid(tcp, owner.pid, pid_type);
	tprint_struct_end();
}

static int
print_fcntl(struct tcb *tcp)
{
	const unsigned int cmd = tcp->u_arg[1];

	switch (cmd) {
	case F_SETFD:
		tprints_arg_next_name("arg");
		printflags(fdflags, tcp->u_arg[2], "FD_???");
		break;
	case F_SETOWN:
		tprints_arg_next_name("arg");
		printpid_tgid_pgid(tcp, tcp->u_arg[2]);
		break;
	case F_SETPIPE_SZ:
		tprints_arg_next_name("arg");
		PRINT_VAL_D(tcp->u_arg[2]);
		break;
	case F_DUPFD:
	case F_DUPFD_CLOEXEC:
		tprints_arg_next_name("arg");
		PRINT_VAL_D(tcp->u_arg[2]);
		return RVAL_DECODED | RVAL_FD;
	case F_SETFL:
		tprints_arg_next_name("arg");
		tprint_open_modes(tcp->u_arg[2]);
		break;
	case F_SETLK:
	case F_SETLKW:
		tprints_arg_next_name("arg");
		printflock(tcp, tcp->u_arg[2], 0);
		break;
	case F_OFD_SETLK:
	case F_OFD_SETLKW:
		tprints_arg_next_name("arg");
		printflock64(tcp, tcp->u_arg[2], 0);
		break;
	case F_SETOWN_EX:
		tprints_arg_next_name("arg");
		print_f_owner_ex(tcp, tcp->u_arg[2]);
		break;
	case F_NOTIFY:
		tprints_arg_next_name("arg");
		printflags64(notifyflags, tcp->u_arg[2], "DN_???");
		break;
	case F_DUPFD_QUERY:
		tprints_arg_next_name("arg");
		printfd(tcp, tcp->u_arg[2]);
		break;
	case F_SETLEASE:
		tprints_arg_next_name("arg");
		printxval64(lockfcmds, tcp->u_arg[2], "F_???");
		break;
	case F_ADD_SEALS:
		tprints_arg_next_name("arg");
		printflags64(f_seals, tcp->u_arg[2], "F_SEAL_???");
		break;
	case F_SETSIG:
		tprints_arg_next_name("arg");
		printsignal(tcp->u_arg[2]);
		break;
	case F_GETOWN:
		return RVAL_DECODED |
		       ((int) tcp->u_rval < 0 ? RVAL_PGID : RVAL_TGID);
	case F_GETPIPE_SZ:
		break;
	case F_GETFD:
		if (entering(tcp) || syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("flags ", fdflags,
					  (kernel_ulong_t) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GETFL:
		if (entering(tcp) || syserror(tcp))
			return 0;
		tcp->auxstr = sprint_open_modes(tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GETLK:
		if (entering(tcp))
			return 0;
		tprints_arg_next_name("arg");
		printflock(tcp, tcp->u_arg[2], 1);
		break;
	case F_OFD_GETLK:
		if (entering(tcp))
			return 0;
		tprints_arg_next_name("arg");
		printflock64(tcp, tcp->u_arg[2], 1);
		break;
	case F_GETOWN_EX:
		if (entering(tcp))
			return 0;
		tprints_arg_next_name("arg");
		print_f_owner_ex(tcp, tcp->u_arg[2]);
		break;
	case F_GETLEASE:
		if (entering(tcp) || syserror(tcp))
			return 0;
		tcp->auxstr = xlookup(lockfcmds, (kernel_ulong_t) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GET_SEALS:
		if (entering(tcp) || syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = sprintflags("seals ", f_seals,
					  (kernel_ulong_t) tcp->u_rval);
		return RVAL_HEX | RVAL_STR;
	case F_GETSIG:
		if (entering(tcp) || syserror(tcp) || tcp->u_rval == 0)
			return 0;
		tcp->auxstr = signame(tcp->u_rval);
		return RVAL_STR;
	default:
		tprints_arg_next_name("arg");
		PRINT_VAL_X(tcp->u_arg[2]);
		break;
	}
	return RVAL_DECODED;
}

SYS_FUNC(fcntl)
{
	if (entering(tcp)) {
		/* fd */
		tprints_arg_name("fd");
		printfd(tcp, tcp->u_arg[0]);

		/* cmd */
		tprints_arg_next_name("op");
		printxval(fcntlcmds, tcp->u_arg[1], "F_???");
	}
	return print_fcntl(tcp);
}

SYS_FUNC(fcntl64)
{
	const unsigned int cmd = tcp->u_arg[1];
	if (entering(tcp)) {
		/* fd */
		tprints_arg_name("fd");
		printfd(tcp, tcp->u_arg[0]);

		/* cmd */
		tprints_arg_next_name("op");
		printxval(fcntlcmds, cmd, "F_???");
	}
	switch (cmd) {
		case F_SETLK64:
		case F_SETLKW64:
			tprints_arg_next_name("arg");
			printflock64(tcp, tcp->u_arg[2], 0);
			return RVAL_DECODED;
		case F_GETLK64:
			if (exiting(tcp)) {
				tprints_arg_next_name("arg");
				printflock64(tcp, tcp->u_arg[2], 1);
			}
			return 0;
	}
	return print_fcntl(tcp);
}
