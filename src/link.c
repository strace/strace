/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>
#include "xlat/at_flags.h"

SYS_FUNC(link)
{
	/* oldpath */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* newpath */
	printpath(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(linkat)
{
	/* olddirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* oldpath */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* newdirfd */
	print_dirfd(tcp, tcp->u_arg[2]);
	tprint_arg_next();

	/* newpath */
	printpath(tcp, tcp->u_arg[3]);
	tprint_arg_next();

	/* flags */
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(unlinkat)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* flags */
	printflags(at_flags, tcp->u_arg[2], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(symlinkat)
{
	/* target */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* newdirfd */
	print_dirfd(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* linkpath */
	printpath(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}
