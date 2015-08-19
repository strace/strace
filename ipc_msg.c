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

#include <sys/ipc.h>
#include <sys/msg.h>

#include "xlat/ipc_msg_flags.h"
#include "xlat/resource_flags.h"

extern void tprint_msgbuf(struct tcb *tcp, const long addr, const unsigned long count);

SYS_FUNC(msgget)
{
	if (tcp->u_arg[0])
		tprintf("%#lx, ", tcp->u_arg[0]);
	else
		tprints("IPC_PRIVATE, ");
	if (printflags(resource_flags, tcp->u_arg[1] & ~0777, NULL) != 0)
		tprints("|");
	tprintf("%#lo", tcp->u_arg[1] & 0777);
	return RVAL_DECODED;
}

static void
tprint_msgsnd(struct tcb *tcp, const long addr, const unsigned long count,
	      const unsigned long flags)
{
	tprint_msgbuf(tcp, addr, count);
	printflags(ipc_msg_flags, flags, "MSG_???");
}

SYS_FUNC(msgsnd)
{
	tprintf("%d, ", (int) tcp->u_arg[0]);
	if (indirect_ipccall(tcp)) {
		tprint_msgsnd(tcp, tcp->u_arg[3], tcp->u_arg[1],
			      tcp->u_arg[2]);
	} else {
		tprint_msgsnd(tcp, tcp->u_arg[1], tcp->u_arg[2],
			      tcp->u_arg[3]);
	}
	return RVAL_DECODED;
}

static void
tprint_msgrcv(struct tcb *tcp, const long addr, const unsigned long count,
	      const long msgtyp)
{
	tprint_msgbuf(tcp, addr, count);
	tprintf("%ld, ", msgtyp);
}

SYS_FUNC(msgrcv)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		if (indirect_ipccall(tcp)) {
			struct ipc_wrapper {
				struct msgbuf *msgp;
				long msgtyp;
			} tmp;

			if (umove_or_printaddr(tcp, tcp->u_arg[3], &tmp))
				tprintf(", %lu, ", tcp->u_arg[1]);
			else
				tprint_msgrcv(tcp, (long) tmp.msgp,
					tcp->u_arg[1], tmp.msgtyp);
			printflags(ipc_msg_flags, tcp->u_arg[2], "MSG_???");
		} else {
			tprint_msgrcv(tcp, tcp->u_arg[1],
				tcp->u_arg[2], tcp->u_arg[3]);
			printflags(ipc_msg_flags, tcp->u_arg[4], "MSG_???");
		}
	}
	return 0;
}
