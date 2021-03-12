/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>

SYS_FUNC(mq_open)
{
	/* name */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* flags */
	tprint_open_modes(tcp->u_arg[1]);

	if (tcp->u_arg[1] & O_CREAT) {
		tprint_arg_next();

		/* mode */
		print_numeric_umode_t(tcp->u_arg[2]);
		tprint_arg_next();

		/* attr */
		printmqattr(tcp, tcp->u_arg[3], false);
	}
	return RVAL_DECODED | RVAL_FD;
}

static int
do_mq_timedsend(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	/* mqdes */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* msg_ptr */
	printstrn(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	tprint_arg_next();

	/* msg_len */
	PRINT_VAL_U(tcp->u_arg[2]);
	tprint_arg_next();

	/* msg_prio */
	PRINT_VAL_U((unsigned int) tcp->u_arg[3]);
	tprint_arg_next();

	/* abs_timeout */
	print_ts(tcp, tcp->u_arg[4]);

	return RVAL_DECODED;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(mq_timedsend_time32)
{
	return do_mq_timedsend(tcp, print_timespec32);
}
#endif

SYS_FUNC(mq_timedsend_time64)
{
	return do_mq_timedsend(tcp, print_timespec64);
}

static int
do_mq_timedreceive(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	if (entering(tcp)) {
		/* mqdes */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* msg_ptr */
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstrn(tcp, tcp->u_arg[1], tcp->u_rval);
		tprint_arg_next();

		/* msg_len */
		PRINT_VAL_U(tcp->u_arg[2]);
		tprint_arg_next();

		printnum_int(tcp, tcp->u_arg[3], "%u");
		tprint_arg_next();

		/*
		 * Since the timeout parameter is read by the kernel
		 * on entering syscall, it has to be decoded the same way
		 * whether the syscall has failed or not.
		 */
		temporarily_clear_syserror(tcp);
		/* abs_timeout */
		print_ts(tcp, tcp->u_arg[4]);
		restore_cleared_syserror(tcp);
	}
	return 0;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(mq_timedreceive_time32)
{
	return do_mq_timedreceive(tcp, print_timespec32);
}
#endif

SYS_FUNC(mq_timedreceive_time64)
{
	return do_mq_timedreceive(tcp, print_timespec64);
}

SYS_FUNC(mq_notify)
{
	/* mqdes */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* sevp */
	print_sigevent(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}

SYS_FUNC(mq_getsetattr)
{
	if (entering(tcp)) {
		/* mqdes */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		/* newattr */
		printmqattr(tcp, tcp->u_arg[1], true);
		tprint_arg_next();
	} else {
		/* oldattr */
		printmqattr(tcp, tcp->u_arg[2], true);
	}
	return 0;
}
