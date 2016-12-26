/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993-1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifdef HAVE_STRUCT_USER_DESC

# include <asm/ldt.h>

void
print_user_desc(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct user_desc desc;

	if (umove_or_printaddr(tcp, addr, &desc))
		return;

	tprintf("{entry_number:%d, "
		"base_addr:%#08x, "
		"limit:%d, "
		"seg_32bit:%d, "
		"contents:%d, "
		"read_exec_only:%d, "
		"limit_in_pages:%d, "
		"seg_not_present:%d, "
		"useable:%d}",
		desc.entry_number,
		desc.base_addr,
		desc.limit,
		desc.seg_32bit,
		desc.contents,
		desc.read_exec_only,
		desc.limit_in_pages,
		desc.seg_not_present,
		desc.useable);
}

SYS_FUNC(modify_ldt)
{
	tprintf("%" PRI_kld ", ", tcp->u_arg[0]);
	if (tcp->u_arg[2] != sizeof(struct user_desc))
		printaddr(tcp->u_arg[1]);
	else
		print_user_desc(tcp, tcp->u_arg[1]);
	tprintf(", %" PRI_klu, tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(set_thread_area)
{
	if (entering(tcp)) {
		print_user_desc(tcp, tcp->u_arg[0]);
	} else {
		struct user_desc desc;

		if (!verbose(tcp) || syserror(tcp) ||
		    umove(tcp, tcp->u_arg[0], &desc) < 0) {
			/* returned entry_number is not available */
		} else {
			static char outstr[32];

			sprintf(outstr, "entry_number:%d", desc.entry_number);
			tcp->auxstr = outstr;
			return RVAL_STR;
		}
	}
	return 0;
}

SYS_FUNC(get_thread_area)
{
	if (exiting(tcp))
		print_user_desc(tcp, tcp->u_arg[0]);
	return 0;
}

#endif /* HAVE_STRUCT_USER_DESC */

#if defined(M68K) || defined(MIPS)
SYS_FUNC(set_thread_area)
{
	printaddr(tcp->u_arg[0]);

	return RVAL_DECODED;

}
#endif

#if defined(M68K)
SYS_FUNC(get_thread_area)
{
	return RVAL_DECODED | RVAL_HEX;
}
#endif
