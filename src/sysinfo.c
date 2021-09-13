/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include DEF_MPERS_TYPE(sysinfo_t)
#include <sys/sysinfo.h>
typedef struct sysinfo sysinfo_t;
#include MPERS_DEFS

SYS_FUNC(sysinfo)
{
	sysinfo_t si;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &si)) {
		tprint_struct_begin();
		PRINT_FIELD_U(si, uptime);
		tprint_struct_next();
		PRINT_FIELD_ARRAY(si, loads, tcp,
				  print_uint_array_member);
		tprint_struct_next();
		PRINT_FIELD_U(si, totalram);
		tprint_struct_next();
		PRINT_FIELD_U(si, freeram);
		tprint_struct_next();
		PRINT_FIELD_U(si, sharedram);
		tprint_struct_next();
		PRINT_FIELD_U(si, bufferram);
		tprint_struct_next();
		PRINT_FIELD_U(si, totalswap);
		tprint_struct_next();
		PRINT_FIELD_U(si, freeswap);
		tprint_struct_next();
		PRINT_FIELD_U(si, procs);
		tprint_struct_next();
		PRINT_FIELD_U(si, totalhigh);
		tprint_struct_next();
		PRINT_FIELD_U(si, freehigh);
		tprint_struct_next();
		PRINT_FIELD_U(si, mem_unit);
		tprint_struct_end();
	}

	return 0;
}
