/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 *
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

#ifdef HAVE_ELF_H
# include <elf.h>
#endif

#include "xlat/nt_descriptor_types.h"

#include "regs.h"
#include "ptrace.h"
#include "xlat/ptrace_cmds.h"
#include "xlat/ptrace_setoptions_flags.h"

#define uoff(member)	offsetof(struct user, member)
#define XLAT_UOFF(member)	{ uoff(member), "offsetof(struct user, " #member ")" }

static const struct xlat struct_user_offsets[] = {
#include "userent.h"
	XLAT_END
};

SYS_FUNC(ptrace)
{
	const struct xlat *x;
	unsigned long addr;

	if (entering(tcp)) {
		printxval(ptrace_cmds, tcp->u_arg[0], "PTRACE_???");
		tprintf(", %lu, ", tcp->u_arg[1]);

		addr = tcp->u_arg[2];
		if (tcp->u_arg[0] == PTRACE_PEEKUSER
		 || tcp->u_arg[0] == PTRACE_POKEUSER
		) {
			for (x = struct_user_offsets; x->str; x++) {
				if (x->val >= addr)
					break;
			}
			if (!x->str)
				tprintf("%#lx, ", addr);
			else if (x->val > addr && x != struct_user_offsets) {
				x--;
				tprintf("%s + %ld, ", x->str, addr - x->val);
			}
			else
				tprintf("%s, ", x->str);
		} else
		if (tcp->u_arg[0] == PTRACE_GETREGSET
		 || tcp->u_arg[0] == PTRACE_SETREGSET
		) {
			printxval(nt_descriptor_types, tcp->u_arg[2], "NT_???");
			tprints(", ");
		} else
			tprintf("%#lx, ", addr);


		switch (tcp->u_arg[0]) {
#ifndef IA64
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			break;
#endif
		case PTRACE_CONT:
		case PTRACE_SINGLESTEP:
		case PTRACE_SYSCALL:
		case PTRACE_DETACH:
			printsignal(tcp->u_arg[3]);
			break;
		case PTRACE_SETOPTIONS:
			printflags(ptrace_setoptions_flags, tcp->u_arg[3], "PTRACE_O_???");
			break;
		case PTRACE_SETSIGINFO: {
			printsiginfo_at(tcp, tcp->u_arg[3]);
			break;
		}
		case PTRACE_SETREGSET:
			tprint_iov(tcp, /*len:*/ 1, tcp->u_arg[3], /*as string:*/ 0);
			break;
		case PTRACE_GETSIGINFO:
		case PTRACE_GETREGSET:
			/* Don't print anything, do it at syscall return. */
			break;
		default:
			tprintf("%#lx", tcp->u_arg[3]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
#ifdef IA64
			return RVAL_HEX;
#else
			printnum_long(tcp, tcp->u_arg[3], "%#lx");
			break;
#endif
		case PTRACE_GETSIGINFO: {
			printsiginfo_at(tcp, tcp->u_arg[3]);
			break;
		}
		case PTRACE_GETREGSET:
			tprint_iov(tcp, /*len:*/ 1, tcp->u_arg[3], /*as string:*/ 0);
			break;
		}
	}
	return 0;
}
