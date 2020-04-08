/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_timeval_utimes(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(futimesat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_timeval_utimes(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}

static int
do_utimensat(struct tcb *const tcp, const print_obj_by_addr_fn print_ts)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_ts(tcp, tcp->u_arg[2]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(utimensat_time32)
{
	return do_utimensat(tcp, print_timespec32_utime_pair);
}
#endif

SYS_FUNC(utimensat_time64)
{
	return do_utimensat(tcp, print_timespec64_utime_pair);
}

#ifdef ALPHA
SYS_FUNC(osf_utimes)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_timeval32_utimes(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}
#endif /* ALPHA */
