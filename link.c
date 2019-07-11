/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>

#include "xlat/at_flags.h"

SYS_FUNC(link)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(linkat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_dirfd(tcp, tcp->u_arg[2]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[3]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(unlinkat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[2], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(symlinkat)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_dirfd(tcp, tcp->u_arg[1]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}
