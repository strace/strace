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
#include "ipc_defs.h"

#ifdef HAVE_SYS_MSG_H
# include <sys/msg.h>
#elif defined HAVE_LINUX_MSG_H
# include <linux/msg.h>
#endif

#include "xlat/ipc_msg_flags.h"
#include "xlat/resource_flags.h"

SYS_FUNC(msgget)
{
	const int key = (int) tcp->u_arg[0];
	if (key)
		tprintf("%#x", key);
	else
		tprints("IPC_PRIVATE");
	tprints(", ");
	if (printflags(resource_flags, tcp->u_arg[1] & ~0777, NULL) != 0)
		tprints("|");
	print_numeric_umode_t(tcp->u_arg[1] & 0777);
	return RVAL_DECODED;
}

static void
tprint_msgsnd(struct tcb *const tcp, const kernel_ulong_t addr,
	      const kernel_ulong_t count, const unsigned int flags)
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
tprint_msgrcv(struct tcb *const tcp, const kernel_ulong_t addr,
	      const kernel_ulong_t count, const kernel_ulong_t msgtyp)
{
	tprint_msgbuf(tcp, addr, count);
	tprintf("%" PRI_kld ", ", msgtyp);
}

static int
fetch_msgrcv_args(struct tcb *const tcp, const kernel_ulong_t addr,
		  kernel_ulong_t *const pair)
{
	if (current_wordsize == sizeof(*pair)) {
		if (umoven_or_printaddr(tcp, addr, 2 * sizeof(*pair), pair))
			return -1;
	} else {
		unsigned int tmp[2];

		if (umove_or_printaddr(tcp, addr, &tmp))
			return -1;
		pair[0] = tmp[0];
		pair[1] = tmp[1];
	}
	return 0;
}

SYS_FUNC(msgrcv)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		if (indirect_ipccall(tcp)) {
			const bool direct =
#ifdef SPARC64
				current_wordsize == 8 ||
#endif
				get_tcb_priv_ulong(tcp) != 0;
			if (direct) {
				tprint_msgrcv(tcp, tcp->u_arg[3],
					      tcp->u_arg[1], tcp->u_arg[4]);
			} else {
				kernel_ulong_t pair[2];

				if (fetch_msgrcv_args(tcp, tcp->u_arg[3], pair))
					tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
				else
					tprint_msgrcv(tcp, pair[0],
						      tcp->u_arg[1], pair[1]);
			}
			printflags(ipc_msg_flags, tcp->u_arg[2], "MSG_???");
		} else {
			tprint_msgrcv(tcp, tcp->u_arg[1],
				tcp->u_arg[2], tcp->u_arg[3]);
			printflags(ipc_msg_flags, tcp->u_arg[4], "MSG_???");
		}
	}
	return 0;
}
