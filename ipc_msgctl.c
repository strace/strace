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

#include DEF_MPERS_TYPE(msqid_ds_t)

#include "ipc_defs.h"

#ifdef HAVE_SYS_MSG_H
/* The C library generally exports the struct the current kernel expects. */
# include <sys/msg.h>
typedef struct msqid_ds msqid_ds_t;
#elif defined HAVE_LINUX_MSG_H
/* The linux header might provide the right struct. */
# include <linux/msg.h>
typedef struct msqid64_ds msqid_ds_t;
#endif

#include MPERS_DEFS

#include "xlat/msgctl_flags.h"

static void
print_msqid_ds(struct tcb *const tcp, const kernel_ulong_t addr, int cmd)
{
	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;
	msqid_ds_t msqid_ds;
	switch (cmd) {
	case IPC_SET:
	case IPC_STAT:
		if (umove_or_printaddr(tcp, addr, &msqid_ds))
			return;

		tprints("{msg_perm={");
		printuid("uid=", msqid_ds.msg_perm.uid);
		printuid(", gid=", msqid_ds.msg_perm.gid);
		tprints(", mode=");
		print_numeric_umode_t(msqid_ds.msg_perm.mode);

		if (cmd != IPC_STAT) {
			tprints("}, ...}");
			break;
		}

		tprintf(", key=%u", (unsigned) msqid_ds.msg_perm.__key);
		printuid(", cuid=", msqid_ds.msg_perm.cuid);
		printuid(", cgid=", msqid_ds.msg_perm.cgid);
		tprints("}");
		tprintf(", msg_stime=%u", (unsigned) msqid_ds.msg_stime);
		tprintf(", msg_rtime=%u", (unsigned) msqid_ds.msg_rtime);
		tprintf(", msg_ctime=%u", (unsigned) msqid_ds.msg_ctime);
		tprintf(", msg_qnum=%u", (unsigned) msqid_ds.msg_qnum);
		tprintf(", msg_qbytes=%u", (unsigned) msqid_ds.msg_qbytes);
		tprintf(", msg_lspid=%u", (unsigned) msqid_ds.msg_lspid);
		tprintf(", msg_lrpid=%u", (unsigned) msqid_ds.msg_lrpid);
		tprints("}");
		break;

	default:
		printaddr(addr);
		break;
	}
}

SYS_FUNC(msgctl)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		PRINTCTL(msgctl_flags, tcp->u_arg[1], "MSG_???");
		tprints(", ");
	} else {
		const kernel_ulong_t addr = tcp->u_arg[indirect_ipccall(tcp) ? 3 : 2];
		print_msqid_ds(tcp, addr, tcp->u_arg[1]);
	}
	return 0;
}
