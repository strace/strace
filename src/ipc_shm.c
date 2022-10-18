/*
 * Copyright (c) 1993 Ulrich Pegelow <pegelow@moorea.uni-muenster.de>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2003-2006 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2022 The strace developers.
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
	/* key */
	printxval(ipc_private, (unsigned int) tcp->u_arg[0], NULL);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* shmflg */
	unsigned int flags = tcp->u_arg[2] & ~0777;
	const unsigned int mask = SHM_HUGE_MASK << SHM_HUGE_SHIFT;
	const unsigned int hugetlb_value = flags & mask;

	flags &= ~mask;
	tprint_flags_begin();
	if (flags || !hugetlb_value)
		printflags_in(shm_resource_flags, flags, NULL);

	if (hugetlb_value) {
		if (flags)
			tprint_flags_or();
		tprint_shift_begin();
		PRINT_VAL_U(hugetlb_value >> SHM_HUGE_SHIFT);
		tprint_shift();
		print_xlat_u(SHM_HUGE_SHIFT);
		tprint_shift_end();
	}

	if (flags || hugetlb_value)
		tprint_flags_or();
	print_numeric_umode_t(tcp->u_arg[2] & 0777);
	tprint_flags_end();

	return RVAL_DECODED;
}

static void
print_shmaddr_shmflg(const kernel_ulong_t shmaddr, const unsigned int shmflg)
{
	/* shmaddr */
	printaddr(shmaddr);
	tprint_arg_next();

	/* shmflg */
	printflags(shm_flags, shmflg, "SHM_???");
}

SYS_FUNC(shmat)
{
	if (entering(tcp)) {
		/* shmid */
		PRINT_VAL_D((int) tcp->u_arg[0]);
		tprint_arg_next();

		if (indirect_ipccall(tcp))
			print_shmaddr_shmflg(tcp->u_arg[3], tcp->u_arg[1]);
		else
			print_shmaddr_shmflg(tcp->u_arg[1], tcp->u_arg[2]);
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
	/* shmaddr */
	printaddr(tcp->u_arg[indirect_ipccall(tcp) ? 3 : 0]);
	return RVAL_DECODED;
}
