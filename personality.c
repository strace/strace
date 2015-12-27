/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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
#include <linux/personality.h>
#include "xlat/personality_types.h"
#include "xlat/personality_flags.h"


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
	const char *type = xlookup(personality_types, pers & PER_MASK);
	char *p;
	static char outstr[1024];
	if (type)
		p = stpcpy(outstr, type);
	else
		p = outstr + sprintf(outstr, "%#x /* %s */", pers & PER_MASK, "PER_???");
	pers &= ~PER_MASK;
	if (pers)
		strcpy(p, sprintflags("|", personality_flags, pers));
	tcp->auxstr = outstr;
	return RVAL_HEX | RVAL_STR;
}
