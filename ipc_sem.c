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

#ifdef HAVE_SYS_SEM_H
# include <sys/sem.h>
#elif defined HAVE_LINUX_SEM_H
# include <linux/sem.h>
#endif

#include "xlat/semctl_flags.h"
#include "xlat/semop_flags.h"

#if defined HAVE_SYS_SEM_H || defined HAVE_LINUX_SEM_H
static bool
print_sembuf(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct sembuf *sb = elem_buf;

	tprintf("{%u, %d, ", sb->sem_num, sb->sem_op);
	printflags(semop_flags, (unsigned short) sb->sem_flg, "SEM_???");
	tprints("}");

	return true;
}
#endif

static void
tprint_sembuf_array(struct tcb *const tcp, const kernel_ulong_t addr,
		    const unsigned int count)
{
#if defined HAVE_SYS_SEM_H || defined HAVE_LINUX_SEM_H
	struct sembuf sb;
	print_array(tcp, addr, count, &sb, sizeof(sb),
		    umoven_or_printaddr, print_sembuf, 0);
#else
	printaddr(addr);
#endif
	tprintf(", %u", count);
}

SYS_FUNC(semop)
{
	tprintf("%d, ", (int) tcp->u_arg[0]);
	if (indirect_ipccall(tcp)) {
		tprint_sembuf_array(tcp, tcp->u_arg[3], tcp->u_arg[1]);
	} else {
		tprint_sembuf_array(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return RVAL_DECODED;
}

SYS_FUNC(semtimedop)
{
	tprintf("%d, ", (int) tcp->u_arg[0]);
	if (indirect_ipccall(tcp)) {
		tprint_sembuf_array(tcp, tcp->u_arg[3], tcp->u_arg[1]);
		tprints(", ");
#if defined(S390) || defined(S390X)
		print_timespec(tcp, tcp->u_arg[2]);
#else
		print_timespec(tcp, tcp->u_arg[4]);
#endif
	} else {
		tprint_sembuf_array(tcp, tcp->u_arg[1], tcp->u_arg[2]);
		tprints(", ");
		print_timespec(tcp, tcp->u_arg[3]);
	}
	return RVAL_DECODED;
}

SYS_FUNC(semget)
{
	const int key = (int) tcp->u_arg[0];
	if (key)
		tprintf("%#x", key);
	else
		tprints("IPC_PRIVATE");
	tprintf(", %d, ", (int) tcp->u_arg[1]);
	if (printflags(resource_flags, tcp->u_arg[2] & ~0777, NULL) != 0)
		tprints("|");
	print_numeric_umode_t(tcp->u_arg[2] & 0777);
	return RVAL_DECODED;
}

SYS_FUNC(semctl)
{
	tprintf("%d, %d, ", (int) tcp->u_arg[0], (int) tcp->u_arg[1]);
	PRINTCTL(semctl_flags, tcp->u_arg[2], "SEM_???");
	tprints(", ");
	if (indirect_ipccall(tcp)
#ifdef SPARC64
	    && current_personality != 0
#endif
	   ) {
		printnum_ptr(tcp, tcp->u_arg[3]);
	} else {
		printaddr(tcp->u_arg[3]);
	}
	return RVAL_DECODED;
}
