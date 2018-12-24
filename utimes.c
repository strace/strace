/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
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
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_timeval_utimes(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(utimensat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_timespec_utime_pair(tcp, tcp->u_arg[2]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[3], "AT_???");

	return RVAL_DECODED;
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
