/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2006 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2006 Bernhard Kaindl <bk@suse.de>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#include <fcntl.h>
#include <sys/stat.h>

#ifdef MAJOR_IN_SYSMACROS
# include <sys/sysmacros.h>
#endif

#ifdef MAJOR_IN_MKDEV
# include <sys/mkdev.h>
#endif

static void
decode_mknod(struct tcb *tcp, int offset)
{
	int mode = tcp->u_arg[offset + 1];

	printpath(tcp, tcp->u_arg[offset]);
	tprintf(", %s", sprintmode(mode));
	switch (mode & S_IFMT) {
	case S_IFCHR:
	case S_IFBLK:
#if defined(SPARC) || defined(SPARC64)
		if (current_personality == 1)
			tprintf(", makedev(%lu, %lu)",
			(unsigned long) ((tcp->u_arg[offset + 2] >> 18) & 0x3fff),
			(unsigned long) (tcp->u_arg[offset + 2] & 0x3ffff));
		else
#endif /* SPARC || SPARC64 */
			tprintf(", makedev(%lu, %lu)",
			(unsigned long) major(tcp->u_arg[offset + 2]),
			(unsigned long) minor(tcp->u_arg[offset + 2]));
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
	print_dirfd(tcp, tcp->u_arg[0]);
	decode_mknod(tcp, 1);

	return RVAL_DECODED;
}
