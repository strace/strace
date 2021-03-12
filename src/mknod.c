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

#include <fcntl.h>
#include <sys/stat.h>

static void
decode_mknod(struct tcb *tcp, int offset)
{
	unsigned short mode = tcp->u_arg[offset + 1];
	unsigned int dev;

	/* pathname */
	printpath(tcp, tcp->u_arg[offset]);
	tprint_arg_next();

	/* mode */
	print_symbolic_mode_t(mode);

	switch (mode & S_IFMT) {
	case S_IFCHR:
	case S_IFBLK:
		dev = tcp->u_arg[offset + 2];
		tprint_arg_next();

		/* dev */
		print_dev_t(dev);
		break;
	}
}

SYS_FUNC(mknod)
{
	decode_mknod(tcp, 0);

	return RVAL_DECODED;
}

SYS_FUNC(mknodat)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	decode_mknod(tcp, 1);

	return RVAL_DECODED;
}
