/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>

SYS_FUNC(mq_open)
{
	/* name */
	tprints_arg_name("name");
	printpath(tcp, tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	tprint_open_modes(tcp->u_arg[1]);

	if (tcp->u_arg[1] & O_CREAT) {
		/* mode */
		tprints_arg_next_name("mode");
		print_numeric_umode_t(tcp->u_arg[2]);

		/* attr */
		tprints_arg_next_name("attr");
		printmqattr(tcp, tcp->u_arg[3], false);
	}
	return RVAL_DECODED | RVAL_FD;
}

static int
do_mq_timedsend(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	/* mqdes */
	tprints_arg_name("mqdes");
	printfd(tcp, tcp->u_arg[0]);

	/* msg_ptr */
	tprints_arg_next_name("msg_ptr");
	printstrn(tcp, tcp->u_arg[1], tcp->u_arg[2]);

	/* msg_len */
	tprints_arg_next_name("msg_len");
	PRINT_VAL_U(tcp->u_arg[2]);

	/* msg_prio */
	tprints_arg_next_name("msg_prio");
	PRINT_VAL_U((unsigned int) tcp->u_arg[3]);

	/* abs_timeout */
	tprints_arg_next_name("abs_timeout");
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
		tprints_arg_name("mqdes");
		printfd(tcp, tcp->u_arg[0]);
	} else {
		/* msg_ptr */
		tprints_arg_next_name("msg_ptr");
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			printstrn(tcp, tcp->u_arg[1], tcp->u_rval);

		/* msg_len */
		tprints_arg_next_name("msg_len");
		PRINT_VAL_U(tcp->u_arg[2]);

		tprints_arg_next_name("msg_prio");
		printnum_int(tcp, tcp->u_arg[3], "%u");

		/*
		 * Since the timeout parameter is read by the kernel
		 * on entering syscall, it has to be decoded the same way
		 * whether the syscall has failed or not.
		 */
		temporarily_clear_syserror(tcp);
		/* abs_timeout */
		tprints_arg_next_name("abs_timeout");
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
	tprints_arg_name("mqdes");
	printfd(tcp, tcp->u_arg[0]);

	/* sevp */
	tprints_arg_next_name("sevp");
	print_sigevent(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}

SYS_FUNC(mq_getsetattr)
{
	if (entering(tcp)) {
		/* mqdes */
		tprints_arg_name("mqdes");
		printfd(tcp, tcp->u_arg[0]);

		/* newattr */
		tprints_arg_next_name("newattr");
		printmqattr(tcp, tcp->u_arg[1], true);
	} else {
		/* oldattr */
		tprints_arg_next_name("oldattr");
		printmqattr(tcp, tcp->u_arg[2], true);
	}
	return 0;
}
