/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 2006-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static void
PRINT_TIMEX_TIME(const typeof_field(TIMEX_T, time) *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*p, tv_sec);
	tprint_struct_next();
	PRINT_FIELD_U(*p, tv_usec);
	tprint_struct_end();
}

int
PRINT_TIMEX(struct tcb *const tcp, const kernel_ulong_t addr)
{
	TIMEX_T tx;

	if (umove_or_printaddr(tcp, addr, &tx))
		return -1;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(tx, modes, adjtimex_modes, "ADJ_???");
	tprint_struct_next();
	PRINT_FIELD_D(tx, offset);
	tprint_struct_next();
	PRINT_FIELD_D(tx, freq);
	tprint_struct_next();
	PRINT_FIELD_D(tx, maxerror);
	tprint_struct_next();
	PRINT_FIELD_D(tx, esterror);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(tx, status, adjtimex_status, "STA_???");
	tprint_struct_next();
	PRINT_FIELD_D(tx, constant);
	tprint_struct_next();
	PRINT_FIELD_D(tx, precision);
	tprint_struct_next();
	PRINT_FIELD_D(tx, tolerance);
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(tx, time, PRINT_TIMEX_TIME);
	tprint_struct_next();
	PRINT_FIELD_D(tx, tick);
	tprint_struct_next();
	PRINT_FIELD_D(tx, ppsfreq);
	tprint_struct_next();
	PRINT_FIELD_D(tx, jitter);
	tprint_struct_next();
	PRINT_FIELD_D(tx, shift);
	tprint_struct_next();
	PRINT_FIELD_D(tx, stabil);
	tprint_struct_next();
	PRINT_FIELD_D(tx, jitcnt);
	tprint_struct_next();
	PRINT_FIELD_D(tx, calcnt);
	tprint_struct_next();
	PRINT_FIELD_D(tx, errcnt);
	tprint_struct_next();
	PRINT_FIELD_D(tx, stbcnt);
	tprint_struct_next();
	PRINT_FIELD_D(tx, tai);
	tprint_struct_end();
	return 0;
}
