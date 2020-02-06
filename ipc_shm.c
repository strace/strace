/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ipc_defs.h"

#include SHM_H_PROVIDER

#ifndef SHM_HUGE_SHIFT
# define SHM_HUGE_SHIFT 26
#endif

#ifndef SHM_HUGE_MASK
# define SHM_HUGE_MASK 0x3f
#endif

#include "xlat/shm_resource_flags.h"
#include "xlat/shm_flags.h"

SYS_FUNC(shmget)
{
	printxval(ipc_private, (unsigned int) tcp->u_arg[0], NULL);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);

	unsigned int flags = tcp->u_arg[2] & ~0777;
	const unsigned int mask = SHM_HUGE_MASK << SHM_HUGE_SHIFT;
	const unsigned int hugetlb_value = flags & mask;

	flags &= ~mask;
	if (flags || !hugetlb_value)
		printflags(shm_resource_flags, flags, NULL);

	if (hugetlb_value) {
		tprintf("%s%u<<",
			flags ? "|" : "",
			hugetlb_value >> SHM_HUGE_SHIFT);
		print_xlat_u(SHM_HUGE_SHIFT);
	}

	if (flags || hugetlb_value)
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
