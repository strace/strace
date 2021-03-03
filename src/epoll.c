/*
 * Copyright (c) 2004-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include <sys/epoll.h>

SYS_FUNC(epoll_create)
{
	/* size */
	int size = tcp->u_arg[0];
	PRINT_VAL_D(size);

	return RVAL_DECODED | RVAL_FD;
}

#include "xlat/epollflags.h"

SYS_FUNC(epoll_create1)
{
	/* flags */
	printflags(epollflags, tcp->u_arg[0], "EPOLL_???");

	return RVAL_DECODED | RVAL_FD;
}

#include "xlat/epollevents.h"

static void
print_epoll_data(const epoll_data_t data)
{
	/*
	 * We cannot know what format the tracee uses, so
	 * print both u32 and u66 which will cover every value.
	 */
	tprint_struct_begin();
	PRINT_FIELD_U(data, u32);
	tprint_struct_next();
	PRINT_FIELD_U(data, u64);
	tprint_struct_end();
}

static bool
print_epoll_event(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct epoll_event *ev = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*ev, events, epollevents, "EPOLL???");
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(*ev, data, print_epoll_data);
	tprint_struct_end();

	return true;
}

#include "xlat/epollctls.h"

SYS_FUNC(epoll_ctl)
{
	/* epfd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* op */
	const unsigned int op = tcp->u_arg[1];
	printxval(epollctls, op, "EPOLL_CTL_???");
	tprint_arg_next();

	/* fd */
	printfd(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* event */
	struct epoll_event ev;
	if (EPOLL_CTL_DEL == op)
		printaddr(tcp->u_arg[3]);
	else if (!umove_or_printaddr(tcp, tcp->u_arg[3], &ev))
		print_epoll_event(tcp, &ev, sizeof(ev), 0);

	return RVAL_DECODED;
}

static void
epoll_wait_common(struct tcb *tcp, const print_obj_by_addr_fn print_timeout)
{
	if (entering(tcp)) {
		/* epfd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* events */
		struct epoll_event ev;
		print_array(tcp, tcp->u_arg[1], tcp->u_rval, &ev, sizeof(ev),
			    tfetch_mem, print_epoll_event, 0);
		tprint_arg_next();

		/* maxevents */
		int maxevents = tcp->u_arg[2];
		PRINT_VAL_D(maxevents);
		tprint_arg_next();

		/* timeout */
		print_timeout(tcp, tcp->u_arg[3]);
	}
}

static int
print_timeout_int(struct tcb *tcp, kernel_ulong_t arg)
{
	int timeout = arg;
	PRINT_VAL_D(timeout);
	return 0;
}

SYS_FUNC(epoll_wait)
{
	epoll_wait_common(tcp, print_timeout_int);
	return 0;
}

static int
epoll_pwait_common(struct tcb *tcp, const print_obj_by_addr_fn print_timeout)
{
	epoll_wait_common(tcp, print_timeout);
	if (exiting(tcp)) {
		tprint_arg_next();
		/* sigmask */
		/* NB: kernel requires arg[5] == NSIG_BYTES */
		print_sigset_addr_len(tcp, tcp->u_arg[4], tcp->u_arg[5]);
		tprint_arg_next();

		/* sigsetsize */
		PRINT_VAL_U(tcp->u_arg[5]);
	}
	return 0;
}

SYS_FUNC(epoll_pwait)
{
	return epoll_pwait_common(tcp, print_timeout_int);
}

SYS_FUNC(epoll_pwait2)
{
	return epoll_pwait_common(tcp, print_timespec64);
}
