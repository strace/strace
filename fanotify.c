/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
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
	unsigned int flags = tcp->u_arg[0];

	printxval(fan_classes, flags & FAN_ALL_CLASS_BITS, "FAN_CLASS_???");
	flags &= ~FAN_ALL_CLASS_BITS;
	if (flags) {
		tprints("|");
		printflags(fan_init_flags, flags, "FAN_???");
	}
	tprints(", ");
	tprint_open_modes((unsigned) tcp->u_arg[1]);

	return RVAL_DECODED | RVAL_FD;
}

#include "xlat/fan_mark_flags.h"
#include "xlat/fan_event_flags.h"

SYS_FUNC(fanotify_mark)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(fan_mark_flags, tcp->u_arg[1], "FAN_MARK_???");
	tprints(", ");
	/*
	 * the mask argument is defined as 64-bit,
	 * but kernel uses the lower 32 bits only.
	 */
	unsigned long long mask = 0;
	int argn = getllval(tcp, &mask, 2);
#ifdef HPPA
	/* Parsic is weird.  See arch/parisc/kernel/sys_parisc32.c.  */
	mask = (mask << 32) | (mask >> 32);
#endif
	printflags64(fan_event_flags, mask, "FAN_???");
	tprints(", ");
	if ((int) tcp->u_arg[argn] == FAN_NOFD)
		print_xlat_d(FAN_NOFD);
	else
		print_dirfd(tcp, tcp->u_arg[argn]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[argn + 1]);

	return RVAL_DECODED;
}
