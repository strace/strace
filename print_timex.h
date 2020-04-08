/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "print_fields.h"

int
PRINT_TIMEX(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMEX_T tx;

	if (umove_or_printaddr(tcp, addr, &tx))
		return -1;

	PRINT_FIELD_FLAGS("{", tx, modes, adjtimex_modes, "ADJ_???");
	PRINT_FIELD_D(", ", tx, offset);
	PRINT_FIELD_D(", ", tx, freq);
	PRINT_FIELD_D(", ", tx, maxerror);
	PRINT_FIELD_D(", ", tx, esterror);
	PRINT_FIELD_FLAGS(", ", tx, status, adjtimex_status, "STA_???");
	PRINT_FIELD_D(", ", tx, constant);
	PRINT_FIELD_D(", ", tx, precision);
	PRINT_FIELD_D(", ", tx, tolerance);
	PRINT_FIELD_D(", time={", tx.time, tv_sec);
	PRINT_FIELD_U(", ", tx.time, tv_usec);
	PRINT_FIELD_D("}, ", tx, tick);
	PRINT_FIELD_D(", ", tx, ppsfreq);
	PRINT_FIELD_D(", ", tx, jitter);
	PRINT_FIELD_D(", ", tx, shift);
	PRINT_FIELD_D(", ", tx, stabil);
	PRINT_FIELD_D(", ", tx, jitcnt);
	PRINT_FIELD_D(", ", tx, calcnt);
	PRINT_FIELD_D(", ", tx, errcnt);
	PRINT_FIELD_D(", ", tx, stbcnt);
	PRINT_FIELD_D(", ", tx, tai);
	tprints("}");
	return 0;
}
