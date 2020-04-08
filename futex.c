/*
 * Copyright (c) 2002-2003 Roland McGrath  <roland@redhat.com>
 * Copyright (c) 2007-2008 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
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

	printaddr(uaddr);
	tprints(", ");
	printxval(futexops, op, "FUTEX_???");
	switch (cmd) {
	case FUTEX_WAIT:
		tprintf(", %u", val);
		tprints(", ");
		print_ts(tcp, timeout);
		break;
	case FUTEX_LOCK_PI:
		tprints(", ");
		print_ts(tcp, timeout);
		break;
	case FUTEX_WAIT_BITSET:
		tprintf(", %u", val);
		tprints(", ");
		print_ts(tcp, timeout);
		tprints(", ");
		printxval(futexbitset, val3, NULL);
		break;
	case FUTEX_WAKE_BITSET:
		tprintf(", %u", val);
		tprints(", ");
		printxval(futexbitset, val3, NULL);
		break;
	case FUTEX_REQUEUE:
		tprintf(", %u", val);
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		break;
	case FUTEX_CMP_REQUEUE:
	case FUTEX_CMP_REQUEUE_PI:
		tprintf(", %u", val);
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		tprintf(", %u", val3);
		break;
	case FUTEX_WAKE_OP:
		tprintf(", %u", val);
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		tprints(", ");
		if ((val3 >> 28) & FUTEX_OP_OPARG_SHIFT) {
			print_xlat(FUTEX_OP_OPARG_SHIFT);
			tprints("<<28|");
		}
		comment = printxval(futexwakeops, (val3 >> 28) & 0x7, NULL)
			? NULL : "FUTEX_OP_???";
		tprints("<<28");
		tprints_comment(comment);
		tprintf("|%#x<<12|", (val3 >> 12) & 0xfff);
		comment = printxval(futexwakecmps, (val3 >> 24) & 0xf, NULL)
			? NULL : "FUTEX_OP_CMP_???";
		tprints("<<24");
		tprints_comment(comment);
		tprintf("|%#x", val3 & 0xfff);
		break;
	case FUTEX_WAIT_REQUEUE_PI:
		tprintf(", %u", val);
		tprints(", ");
		print_ts(tcp, timeout);
		tprints(", ");
		printaddr(uaddr2);
		break;
	case FUTEX_FD:
	case FUTEX_WAKE:
		tprintf(", %u", val);
		break;
	case FUTEX_UNLOCK_PI:
	case FUTEX_TRYLOCK_PI:
		break;
	default:
		tprintf(", %u", val);
		tprints(", ");
		printaddr(timeout);
		tprints(", ");
		printaddr(uaddr2);
		tprintf(", %#x", val3);
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
