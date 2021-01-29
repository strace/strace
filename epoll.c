/*
 * Copyright (c) 2004-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <fcntl.h>
#include <sys/epoll.h>
#include "print_fields.h"

SYS_FUNC(epoll_create)
{
	tprintf("%d", (int) tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_FD;
}

#include "xlat/epollflags.h"

SYS_FUNC(epoll_create1)
{
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
	tprints("}");
}

static bool
print_epoll_event(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct epoll_event *ev = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*ev, events, epollevents, "EPOLL???");
	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(*ev, data, print_epoll_data);
	tprints("}");

	return true;
}

#include "xlat/epollctls.h"

SYS_FUNC(epoll_ctl)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	const unsigned int op = tcp->u_arg[1];
	printxval(epollctls, op, "EPOLL_CTL_???");
	tprints(", ");
	printfd(tcp, tcp->u_arg[2]);
	tprints(", ");
	struct epoll_event ev;
	if (EPOLL_CTL_DEL == op)
		printaddr(tcp->u_arg[3]);
	else if (!umove_or_printaddr(tcp, tcp->u_arg[3], &ev))
		print_epoll_event(tcp, &ev, sizeof(ev), 0);

	return RVAL_DECODED;
}

static void
epoll_wait_common(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		struct epoll_event ev;
		print_array(tcp, tcp->u_arg[1], tcp->u_rval, &ev, sizeof(ev),
			    tfetch_mem, print_epoll_event, 0);
		tprintf(", %d, %d", (int) tcp->u_arg[2], (int) tcp->u_arg[3]);
	}
}

SYS_FUNC(epoll_wait)
{
	epoll_wait_common(tcp);
	return 0;
}

SYS_FUNC(epoll_pwait)
{
	epoll_wait_common(tcp);
	if (exiting(tcp)) {
		tprints(", ");
		/* NB: kernel requires arg[5] == NSIG_BYTES */
		print_sigset_addr_len(tcp, tcp->u_arg[4], tcp->u_arg[5]);
		tprintf(", %" PRI_klu, tcp->u_arg[5]);
	}
	return 0;
}
