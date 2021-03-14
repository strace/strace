/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

static int
decode_readlink(struct tcb *tcp, int offset)
{
	if (entering(tcp)) {
		/* pathname */
		printpath(tcp, tcp->u_arg[offset]);
		tprint_arg_next();
	} else {
		/* buf */
		if (syserror(tcp))
			printaddr(tcp->u_arg[offset + 1]);
		else
			/* Used to use printpathn(), but readlink
			 * neither includes NUL in the returned count,
			 * nor actually writes it into memory.
			 * printpathn() would decide on printing
			 * "..." continuation based on garbage
			 * past return buffer's end.
			 */
			printstrn(tcp, tcp->u_arg[offset + 1], tcp->u_rval);
		tprint_arg_next();

		/* bufsiz */
		PRINT_VAL_U(tcp->u_arg[offset + 2]);
	}
	return 0;
}

SYS_FUNC(readlink)
{
	return decode_readlink(tcp, 0);
}

SYS_FUNC(readlinkat)
{
	if (entering(tcp)) {
		/* dirfd */
		print_dirfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	}
	return decode_readlink(tcp, 1);
}
