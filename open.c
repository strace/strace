/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2007 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2007 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2009-2013 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xstring.h"

#include <asm/fcntl.h>

/* some libcs are guilty of messing up with O_ACCMODE */
#undef O_ACCMODE
#define O_ACCMODE 03

#ifdef O_LARGEFILE
# if O_LARGEFILE == 0		/* biarch platforms in 64-bit mode */
#  undef O_LARGEFILE
# endif
#endif

#include "xlat/open_access_modes.h"
#include "xlat/open_mode_flags.h"

#ifndef AT_FDCWD
# define AT_FDCWD	-100
#endif

/* The fd is an "int", so when decoding x86 on x86_64, we need to force sign
 * extension to get the right value.  We do this by declaring fd as int here.
 */
void
print_dirfd(struct tcb *tcp, int fd)
{
	if (fd == AT_FDCWD)
		print_xlat_d(AT_FDCWD);
	else
		printfd(tcp, fd);
}

/*
 * low bits of the open(2) flags define access mode,
 * other bits are real flags.
 */
const char *
sprint_open_modes(unsigned int flags)
{
	static char outstr[sizeof("flags O_ACCMODE")];
	char *p;
	char sep;
	const char *str;

	sep = ' ';
	p = stpcpy(outstr, "flags");
	str = xlookup(open_access_modes, flags & 3);
	if (str) {
		*p++ = sep;
		p = stpcpy(p, str);
		flags &= ~3;
		if (!flags)
			return outstr;
		sep = '|';
	}
	*p = '\0';

	return sprintflags_ex(outstr, open_mode_flags, flags, sep,
			      XLAT_STYLE_ABBREV) ?: outstr;
}

void
tprint_open_modes(unsigned int flags)
{
	print_xlat_ex(flags, sprint_open_modes(flags) + sizeof("flags"),
		      XLAT_STYLE_DEFAULT);
}

static int
decode_open(struct tcb *tcp, int offset)
{
	printpath(tcp, tcp->u_arg[offset]);
	tprints(", ");
	/* flags */
	tprint_open_modes(tcp->u_arg[offset + 1]);
	if (tcp->u_arg[offset + 1] & (O_CREAT | __O_TMPFILE)) {
		/* mode */
		tprints(", ");
		print_numeric_umode_t(tcp->u_arg[offset + 2]);
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(open)
{
	return decode_open(tcp, 0);
}

SYS_FUNC(openat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	return decode_open(tcp, 1);
}

SYS_FUNC(creat)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_numeric_umode_t(tcp->u_arg[1]);

	return RVAL_DECODED | RVAL_FD;
}
