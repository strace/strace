/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
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

#ifdef HAVE_SYS_SHM_H
# include <sys/shm.h>
#elif defined HAVE_LINUX_SHM_H
# include <linux/shm.h>
#endif

#include "xlat/shm_resource_flags.h"
#include "xlat/shm_flags.h"

SYS_FUNC(shmget)
{
	const int key = (int) tcp->u_arg[0];
	if (key)
		tprintf("%#x", key);
	else
		tprints("IPC_PRIVATE");
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	if (printflags(shm_resource_flags, tcp->u_arg[2] & ~0777, NULL) != 0)
		tprints("|");
	print_numeric_umode_t(tcp->u_arg[2] & 0777);
	return RVAL_DECODED;
}

SYS_FUNC(shmat)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		if (indirect_ipccall(tcp)) {
			printaddr(tcp->u_arg[3]);
			tprints(", ");
			printflags(shm_flags, tcp->u_arg[1], "SHM_???");
		} else {
			printaddr(tcp->u_arg[1]);
			tprints(", ");
			printflags(shm_flags, tcp->u_arg[2], "SHM_???");
		}
		return 0;
	} else {
		if (syserror(tcp))
			return 0;
		if (indirect_ipccall(tcp)) {
			union {
				uint64_t r64;
				uint32_t r32;
			} u;
			if (umoven(tcp, tcp->u_arg[2], current_wordsize, &u) < 0)
				return RVAL_NONE;
			tcp->u_rval = (sizeof(u.r32) == current_wordsize)
				      ? u.r32 : u.r64;
		}
		return RVAL_HEX;
	}
}

SYS_FUNC(shmdt)
{
	printaddr(tcp->u_arg[indirect_ipccall(tcp) ? 3 : 0]);
	return RVAL_DECODED;
}
