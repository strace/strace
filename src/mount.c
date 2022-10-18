/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#define MS_MGC_VAL	0xc0ed0000	/* old magic mount flag number */
#define MS_MGC_MSK	0xffff0000	/* old magic mount flag mask */

#include "xlat/mount_flags.h"

SYS_FUNC(mount)
{
	bool ignore_type = false;
	bool ignore_data = false;
	bool old_magic = false;
	kernel_ulong_t flags = tcp->u_arg[3];

	/* Discard magic */
	if ((flags & MS_MGC_MSK) == MS_MGC_VAL) {
		flags &= ~MS_MGC_MSK;
		old_magic = true;
	}

	if (flags & MS_REMOUNT)
		ignore_type = true;
	else if (flags & (MS_BIND | MS_MOVE | MS_SHARED
			  | MS_PRIVATE | MS_SLAVE | MS_UNBINDABLE))
		ignore_type = ignore_data = true;

	/* source */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* target */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* filesystemtype */
	if (ignore_type)
		printaddr(tcp->u_arg[2]);
	else
		printstr(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* mountflags */
	tprint_flags_begin();
	if (old_magic) {
		print_xlat(MS_MGC_VAL);
		if (flags)
			tprint_flags_or();
	}
	if (flags || !old_magic)
		printflags64_in(mount_flags, flags, "MS_???");
	tprint_flags_end();
	tprint_arg_next();

	/* data */
	if (ignore_data)
		printaddr(tcp->u_arg[4]);
	else
		printstr(tcp, tcp->u_arg[4]);

	return RVAL_DECODED;
}
