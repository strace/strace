/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007-2015 Dmitry V. Levin <ldv@altlinux.org>
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

	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");

	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");

	if (ignore_type)
		printaddr(tcp->u_arg[2]);
	else
		printstr(tcp, tcp->u_arg[2]);
	tprints(", ");

	if (old_magic) {
		tprints("MS_MGC_VAL");
		if (flags)
			tprints("|");
	}
	if (flags || !old_magic)
		printflags64(mount_flags, flags, "MS_???");
	tprints(", ");

	if (ignore_data)
		printaddr(tcp->u_arg[4]);
	else
		printstr(tcp, tcp->u_arg[4]);

	return RVAL_DECODED;
}
