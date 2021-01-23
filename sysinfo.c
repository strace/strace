/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include DEF_MPERS_TYPE(sysinfo_t)
#include <sys/sysinfo.h>
typedef struct sysinfo sysinfo_t;
#include MPERS_DEFS
#include "print_fields.h"

SYS_FUNC(sysinfo)
{
	sysinfo_t si;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &si)) {
		PRINT_FIELD_U("{", si, uptime);
		PRINT_FIELD_ARRAY(", ", si, loads, tcp,
				  print_kulong_array_member);
		PRINT_FIELD_U(", ", si, totalram);
		PRINT_FIELD_U(", ", si, freeram);
		PRINT_FIELD_U(", ", si, sharedram);
		PRINT_FIELD_U(", ", si, bufferram);
		PRINT_FIELD_U(", ", si, totalswap);
		PRINT_FIELD_U(", ", si, freeswap);
		PRINT_FIELD_U(", ", si, procs);
		PRINT_FIELD_U(", ", si, totalhigh);
		PRINT_FIELD_U(", ", si, freehigh);
		PRINT_FIELD_U(", ", si, mem_unit);
		tprints("}");
	}

	return 0;
}
