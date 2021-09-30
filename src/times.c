/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include DEF_MPERS_TYPE(tms_t)
#include <sys/times.h>
typedef struct tms tms_t;
#include MPERS_DEFS

SYS_FUNC(times)
{
	tms_t tbuf;

	if (exiting(tcp) && !umove_or_printaddr(tcp, tcp->u_arg[0], &tbuf)) {
		tprint_struct_begin();
		PRINT_FIELD_CLOCK_T(tbuf, tms_utime);
		tprint_struct_next();
		PRINT_FIELD_CLOCK_T(tbuf, tms_stime);
		tprint_struct_next();
		PRINT_FIELD_CLOCK_T(tbuf, tms_cutime);
		tprint_struct_next();
		PRINT_FIELD_CLOCK_T(tbuf, tms_cstime);
		tprint_struct_end();
	}

	return 0;
}
