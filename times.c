/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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
		tprintf("{tms_utime=%llu, tms_stime=%llu, ",
			zero_extend_signed_to_ull(tbuf.tms_utime),
			zero_extend_signed_to_ull(tbuf.tms_stime));
		tprintf("tms_cutime=%llu, tms_cstime=%llu}",
			zero_extend_signed_to_ull(tbuf.tms_cutime),
			zero_extend_signed_to_ull(tbuf.tms_cstime));
	}

	return 0;
}
