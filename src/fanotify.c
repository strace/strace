/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/fan_classes.h"
#include "xlat/fan_init_flags.h"

#ifndef FAN_ALL_CLASS_BITS
# define FAN_ALL_CLASS_BITS (FAN_CLASS_NOTIF | FAN_CLASS_CONTENT | FAN_CLASS_PRE_CONTENT)
#endif
#ifndef FAN_NOFD
# define FAN_NOFD -1
#endif

SYS_FUNC(fanotify_init)
{
	/* flags */
	tprints_arg_name("flags");
	unsigned int flags = tcp->u_arg[0];

	tprint_flags_begin();
	printxval(fan_classes, flags & FAN_ALL_CLASS_BITS, "FAN_CLASS_???");
	flags &= ~FAN_ALL_CLASS_BITS;
	if (flags) {
		tprint_flags_or();
		printflags_in(fan_init_flags, flags, "FAN_???");
	}
	tprint_flags_end();

	/* event_f_flags */
	tprints_arg_next_name("event_f_flags");
	tprint_open_modes((unsigned) tcp->u_arg[1]);

	return RVAL_DECODED | RVAL_FD;
}

#include "xlat/fan_mark_flags.h"
#include "xlat/fan_event_flags.h"

SYS_FUNC(fanotify_mark)
{
	/* fanotify_fd */
	tprints_arg_name("fanotify_fd");
	printfd(tcp, tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(fan_mark_flags, tcp->u_arg[1], "FAN_MARK_???");

	/*
	 * the mask argument is defined as 64-bit,
	 * but kernel uses the lower 32 bits only.
	 */
	tprints_arg_next_name("mask");
	unsigned long long mask = 0;
	unsigned int argn = getllval(tcp, &mask, 2);
	printflags64(fan_event_flags, mask, "FAN_???");

	/* dirfd */
	tprints_arg_next_name("dirfd");
	if ((int) tcp->u_arg[argn] == FAN_NOFD)
		print_xlat_d(FAN_NOFD);
	else
		print_dirfd(tcp, tcp->u_arg[argn]);

	/* pathname */
	tprints_arg_next_name("pathname");
	printpath(tcp, tcp->u_arg[argn + 1]);

	return RVAL_DECODED;
}
