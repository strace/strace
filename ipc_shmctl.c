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

#include DEF_MPERS_TYPE(shmid_ds_t)

#include "ipc_defs.h"

#ifdef HAVE_SYS_SHM_H
/* The C library generally exports the struct the current kernel expects. */
# include <sys/shm.h>
typedef struct shmid_ds shmid_ds_t;
#elif defined HAVE_LINUX_SHM_H
/* The linux header might provide the right struct. */
# include <linux/shm.h>
typedef struct shmid64_ds shmid_ds_t;
#endif

#include MPERS_DEFS

#include "xlat/shmctl_flags.h"

static void
print_shmid_ds(struct tcb *const tcp, const kernel_ulong_t addr, int cmd)
{
	/* TODO: We don't properly decode old compat ipc calls. */
	if (cmd & IPC_64)
		cmd &= ~IPC_64;
	shmid_ds_t shmid_ds;
	switch (cmd) {
	case IPC_SET:
	case IPC_STAT:
		if (umove_or_printaddr(tcp, addr, &shmid_ds))
			return;

		tprints("{shm_perm={");
		printuid("uid=", shmid_ds.shm_perm.uid);
		printuid(", gid=", shmid_ds.shm_perm.gid);
		tprints(", mode=");
		print_numeric_umode_t(shmid_ds.shm_perm.mode);

		if (cmd != IPC_STAT) {
			tprints("}, ...}");
			break;
		}

		tprintf(", key=%u", (unsigned) shmid_ds.shm_perm.__key);
		printuid(", cuid=", shmid_ds.shm_perm.cuid);
		printuid(", cgid=", shmid_ds.shm_perm.cgid);
		tprints("}");
		tprintf(", shm_segsz=%u", (unsigned) shmid_ds.shm_segsz);
		tprintf(", shm_cpid=%u", (unsigned) shmid_ds.shm_cpid);
		tprintf(", shm_lpid=%u", (unsigned) shmid_ds.shm_lpid);
		tprintf(", shm_nattch=%u", (unsigned) shmid_ds.shm_nattch);
		tprintf(", shm_atime=%u", (unsigned) shmid_ds.shm_atime);
		tprintf(", shm_dtime=%u", (unsigned) shmid_ds.shm_dtime);
		tprintf(", shm_ctime=%u", (unsigned) shmid_ds.shm_ctime);
		tprints("}");
		break;

	default:
		printaddr(addr);
		break;
	}
}

SYS_FUNC(shmctl)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		PRINTCTL(shmctl_flags, tcp->u_arg[1], "SHM_???");
		tprints(", ");
	} else {
		const kernel_ulong_t addr = tcp->u_arg[indirect_ipccall(tcp) ? 3 : 2];
		print_shmid_ds(tcp, addr, tcp->u_arg[1]);
	}
	return 0;
}
