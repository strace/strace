/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

SYS_FUNC(sysinfo)
{
	sysinfo_t si;

	if (entering(tcp))
		return 0;

	if (!umove_or_printaddr(tcp, tcp->u_arg[0], &si)) {
		tprintf("{uptime=%llu"
			", loads=[%llu, %llu, %llu]"
			", totalram=%llu"
			", freeram=%llu"
			", sharedram=%llu"
			", bufferram=%llu"
			", totalswap=%llu"
			", freeswap=%llu"
			", procs=%u"
			", totalhigh=%llu"
			", freehigh=%llu"
			", mem_unit=%u"
			"}",
			zero_extend_signed_to_ull(si.uptime)
			, zero_extend_signed_to_ull(si.loads[0])
			, zero_extend_signed_to_ull(si.loads[1])
			, zero_extend_signed_to_ull(si.loads[2])
			, zero_extend_signed_to_ull(si.totalram)
			, zero_extend_signed_to_ull(si.freeram)
			, zero_extend_signed_to_ull(si.sharedram)
			, zero_extend_signed_to_ull(si.bufferram)
			, zero_extend_signed_to_ull(si.totalswap)
			, zero_extend_signed_to_ull(si.freeswap)
			, (unsigned) si.procs
			, zero_extend_signed_to_ull(si.totalhigh)
			, zero_extend_signed_to_ull(si.freehigh)
			, si.mem_unit
			);
	}

	return 0;
}
