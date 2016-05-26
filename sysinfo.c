/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2012 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
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
			widen_to_ull(si.uptime)
			, widen_to_ull(si.loads[0])
			, widen_to_ull(si.loads[1])
			, widen_to_ull(si.loads[2])
			, widen_to_ull(si.totalram)
			, widen_to_ull(si.freeram)
			, widen_to_ull(si.sharedram)
			, widen_to_ull(si.bufferram)
			, widen_to_ull(si.totalswap)
			, widen_to_ull(si.freeswap)
			, (unsigned) si.procs
			, widen_to_ull(si.totalhigh)
			, widen_to_ull(si.freehigh)
			, si.mem_unit
			);
	}

	return 0;
}
