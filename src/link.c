/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
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
	tprints_arg_name("oldpath");
	printpath(tcp, tcp->u_arg[0]);

	/* newpath */
	tprints_arg_next_name("newpath");
	printpath(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(linkat)
{
	/* olddirfd */
	tprints_arg_name("olddirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* oldpath */
	tprints_arg_next_name("oldpath");
	printpath(tcp, tcp->u_arg[1]);

	/* newdirfd */
	tprints_arg_next_name("newdirfd");
	print_dirfd(tcp, tcp->u_arg[2]);

	/* newpath */
	tprints_arg_next_name("newpath");
	printpath(tcp, tcp->u_arg[3]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(unlinkat)
{
	/* dirfd */
	tprints_arg_name("dirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprints_arg_next_name("pathname");
	printpath(tcp, tcp->u_arg[1]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(at_flags, tcp->u_arg[2], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(symlinkat)
{
	/* target */
	tprints_arg_name("target");
	printpath(tcp, tcp->u_arg[0]);

	/* newdirfd */
	tprints_arg_next_name("newdirfd");
	print_dirfd(tcp, tcp->u_arg[1]);

	/* linkpath */
	tprints_arg_next_name("linkpath");
	printpath(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}
