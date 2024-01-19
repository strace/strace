/*
 * Copyright (c) 2002-2003 Roland McGrath  <roland@redhat.com>
 * Copyright (c) 2007-2008 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifndef FUTEX_PRIVATE_FLAG
# define FUTEX_PRIVATE_FLAG 128
#endif
#ifndef FUTEX_CLOCK_REALTIME
# define FUTEX_CLOCK_REALTIME 256
#endif
#ifndef FUTEX_OP_OPARG_SHIFT
# define FUTEX_OP_OPARG_SHIFT 8
#endif

#include "xlat/futexbitset.h"
#include "xlat/futexops.h"
#include "xlat/futexwakeops.h"
#include "xlat/futexwakecmps.h"

static int
do_futex(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	const kernel_ulong_t uaddr = tcp->u_arg[0];
	const int op = tcp->u_arg[1];
	const int cmd = op & 127;
	const kernel_ulong_t timeout = tcp->u_arg[3];
	const kernel_ulong_t uaddr2 = tcp->u_arg[4];
	const unsigned int val = tcp->u_arg[2];
	const unsigned int val2 = tcp->u_arg[3];
	const unsigned int val3 = tcp->u_arg[5];
	const char *comment;

	/* uaddr */
	printaddr(uaddr);
	tprint_arg_next();

	/* futex_op */
	printxval(futexops, op, "FUTEX_???");

	switch (cmd) {
	case FUTEX_WAIT:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		print_ts(tcp, timeout);
		break;
	case FUTEX_LOCK_PI:
	case FUTEX_LOCK_PI2:
		tprint_arg_next();
		print_ts(tcp, timeout);
		break;
	case FUTEX_WAIT_BITSET:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		print_ts(tcp, timeout);

		tprint_arg_next();
		printxval(futexbitset, val3, NULL);
		break;
	case FUTEX_WAKE_BITSET:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		printxval(futexbitset, val3, NULL);
		break;
	case FUTEX_REQUEUE:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		PRINT_VAL_U(val2);

		tprint_arg_next();
		printaddr(uaddr2);
		break;
	case FUTEX_CMP_REQUEUE:
	case FUTEX_CMP_REQUEUE_PI:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		PRINT_VAL_U(val2);

		tprint_arg_next();
		printaddr(uaddr2);

		tprint_arg_next();
		PRINT_VAL_U(val3);
		break;
	case FUTEX_WAKE_OP:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		PRINT_VAL_U(val2);

		tprint_arg_next();
		printaddr(uaddr2);

		tprint_arg_next();
		tprint_flags_begin();
		if ((val3 >> 28) & FUTEX_OP_OPARG_SHIFT) {
			tprint_shift_begin();
			print_xlat(FUTEX_OP_OPARG_SHIFT);
			tprint_shift();
			PRINT_VAL_U(28);
			tprint_shift_end();
			tprint_flags_or();
		}
		tprint_shift_begin();
		comment = printxval(futexwakeops, (val3 >> 28) & 0x7, NULL)
			? NULL : "FUTEX_OP_???";
		tprint_shift();
		PRINT_VAL_U(28);
		tprint_shift_end();
		tprints_comment(comment);
		tprint_flags_or();
		tprint_shift_begin();
		PRINT_VAL_X((val3 >> 12) & 0xfff);
		tprint_shift();
		PRINT_VAL_U(12);
		tprint_shift_end();
		tprint_flags_or();
		tprint_shift_begin();
		comment = printxval(futexwakecmps, (val3 >> 24) & 0xf, NULL)
			? NULL : "FUTEX_OP_CMP_???";
		tprint_shift();
		PRINT_VAL_U(24);
		tprint_shift_end();
		tprints_comment(comment);
		tprint_flags_or();
		PRINT_VAL_X(val3 & 0xfff);
		tprint_flags_end();
		break;
	case FUTEX_WAIT_REQUEUE_PI:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		print_ts(tcp, timeout);

		tprint_arg_next();
		printaddr(uaddr2);
		break;
	case FUTEX_FD:
	case FUTEX_WAKE:
		tprint_arg_next();
		PRINT_VAL_U(val);
		break;
	case FUTEX_UNLOCK_PI:
	case FUTEX_TRYLOCK_PI:
		break;
	default:
		tprint_arg_next();
		PRINT_VAL_U(val);

		tprint_arg_next();
		printaddr(timeout);

		tprint_arg_next();
		printaddr(uaddr2);

		tprint_arg_next();
		PRINT_VAL_X(val3);
		break;
	}

	return RVAL_DECODED;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(futex_time32)
{
	return do_futex(tcp, print_timespec32);
}
#endif

SYS_FUNC(futex_time64)
{
	return do_futex(tcp, print_timespec64);
}
