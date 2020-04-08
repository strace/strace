/*
 * Copyright (c) 2001 Wichert Akkerman <wichert@deephackmode.org>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef MIPS

# ifdef HAVE_LINUX_UTSNAME_H
#  include <linux/utsname.h>
# endif
# ifdef HAVE_ASM_SYSMIPS_H
#  include <asm/sysmips.h>
# endif

# ifndef __NEW_UTS_LEN
#  define __NEW_UTS_LEN 64
# endif

# include "xlat/sysmips_operations.h"

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
			print_quoted_cstring(nodename, __NEW_UTS_LEN + 1);
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
