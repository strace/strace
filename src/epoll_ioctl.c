/*
 * Copyright (c) 2024 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/eventpoll.h>

static void
print_struct_epoll_params(struct tcb *const tcp, const kernel_ulong_t addr)
{
	CHECK_IOCTL_SIZE(EPIOCGPARAMS, 8);
	CHECK_IOCTL_SIZE(EPIOCSPARAMS, 8);
	CHECK_TYPE_SIZE(struct epoll_params, 8);
	struct epoll_params ep;

	if (umove_or_printaddr(tcp, addr, &ep))
		return;

	tprint_struct_begin();
	PRINT_FIELD_TICKS(ep, busy_poll_usecs, 1000000, 6);
	tprint_struct_next();
	PRINT_FIELD_U(ep, busy_poll_budget);
	tprint_struct_next();
	PRINT_FIELD_X(ep, prefer_busy_poll);
	if (ep.__pad) {
		tprint_struct_next();
		PRINT_FIELD_X(ep, __pad);
	}
	tprint_struct_end();
}

int
epoll_ioctl(struct tcb *const tcp, const unsigned int code,
	    const kernel_ulong_t arg)
{
	switch (code) {
	case EPIOCSPARAMS:
		tprint_arg_next();
		print_struct_epoll_params(tcp, arg);

		return RVAL_IOCTL_DECODED;

	case EPIOCGPARAMS:
		if (entering(tcp)) {
			tprint_arg_next();
			return 0;
		}
		print_struct_epoll_params(tcp, arg);

		return RVAL_IOCTL_DECODED;

	default:
		return RVAL_DECODED;
	}
}
