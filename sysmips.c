/*
 * Copyright (c) 2001 Wichert Akkerman <wichert@deephackmode.org>
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

#ifdef MIPS

#ifdef HAVE_LINUX_UTSNAME_H
# include <linux/utsname.h>
#endif
#ifdef HAVE_ASM_SYSMIPS_H
# include <asm/sysmips.h>
#endif

#ifndef __NEW_UTS_LEN
# define __NEW_UTS_LEN 64
#endif

#include "xlat/sysmips_operations.h"

SYS_FUNC(sysmips)
{
	printxval64(sysmips_operations, tcp->u_arg[0], "???");
	tprints(", ");

	switch (tcp->u_arg[0]) {
	case SETNAME: {
		char nodename[__NEW_UTS_LEN + 1];

		if (!verbose(tcp))
			break;
		if (umovestr(tcp, tcp->u_arg[1], (__NEW_UTS_LEN + 1),
			     nodename) < 0) {
			printaddr(tcp->u_arg[1]);
		} else {
			print_quoted_string(nodename, __NEW_UTS_LEN + 1,
					    QUOTE_0_TERMINATED);
		}
		return RVAL_DECODED;
	}
	case MIPS_ATOMIC_SET:
		printaddr(tcp->u_arg[1]);
		tprintf(", %#" PRI_klx, tcp->u_arg[2]);
		return RVAL_DECODED;
	case MIPS_FIXADE:
		tprintf("%#" PRI_klx, tcp->u_arg[1]);
		return RVAL_DECODED;
	}

	tprintf("%" PRI_kld ", %" PRI_kld ", %" PRI_kld,
		tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
	return RVAL_DECODED;
}

#endif /* MIPS */
