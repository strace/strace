/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "xlat/personality_flags.h"
#include "xlat/personality_types.h"


SYS_FUNC(personality)
{
	unsigned int pers;

	if (entering(tcp)) {
		pers = tcp->u_arg[0];
		if (0xffffffff == pers) {
			tprints("0xffffffff");
		} else {
			printxval(personality_types, pers & PER_MASK, "PER_???");
			pers &= ~PER_MASK;
			if (pers) {
				tprints("|");
				printflags(personality_flags, pers, NULL);
			}
		}
		return 0;
	}

	if (syserror(tcp))
		return 0;

	pers = tcp->u_rval;
	static char outstr[1024];
	char *p = outstr + sprintxval(outstr, sizeof(outstr), personality_types,
				      pers & PER_MASK, "PER_???");
	pers &= ~PER_MASK;
	if (pers)
		strcpy(p, sprintflags("|", personality_flags, pers));
	tcp->auxstr = outstr;
	return RVAL_HEX | RVAL_STR;
}
