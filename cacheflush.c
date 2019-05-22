/*
 * Copyright (c) 1999 Andreas Schwab <schwab@issan.cs.uni-dortmund.de>
 * Copyright (c) 2010 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2010 Carmelo Amoroso <carmelo.amoroso@st.com>
 * Copyright (c) 2015 Ezequiel Garcia <ezequiel@vanguardiasur.com.ar>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_ASM_CACHECTL_H
# include <asm/cachectl.h>
#endif

#ifdef M68K
# include "xlat/cacheflush_scope.h"

static const struct xlat cacheflush_flags[] = {
# ifdef FLUSH_CACHE_BOTH
	XLAT(FLUSH_CACHE_BOTH),
# endif
# ifdef FLUSH_CACHE_DATA
	XLAT(FLUSH_CACHE_DATA),
# endif
# ifdef FLUSH_CACHE_INSN
	XLAT(FLUSH_CACHE_INSN),
# endif
	XLAT_END
};

SYS_FUNC(cacheflush)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	tprints(", ");
	/* scope */
	printxval(cacheflush_scope, tcp->u_arg[1], "FLUSH_SCOPE_???");
	tprints(", ");
	/* flags */
	printflags(cacheflush_flags, tcp->u_arg[2], "FLUSH_CACHE_???");
	/* len */
	tprintf(", %lu", tcp->u_arg[3]);

	return RVAL_DECODED;
}
#endif /* M68K */

#if defined(BFIN) || defined(CSKY)
static const struct xlat cacheflush_flags[] = {
	XLAT(ICACHE),
	XLAT(DCACHE),
	XLAT(BCACHE),
	XLAT_END
};

SYS_FUNC(cacheflush)
{
	/* start addr */
	printaddr(tcp->u_arg[0]);
	/* length */
	tprintf(", %lu, ", tcp->u_arg[1]);
	/* flags */
	printxval(cacheflush_flags, tcp->u_arg[2], "?CACHE");

	return RVAL_DECODED;
}
#endif /* BFIN || CSKY */

#ifdef SH
static const struct xlat cacheflush_flags[] = {
# ifdef CACHEFLUSH_D_INVAL
	XLAT(CACHEFLUSH_D_INVAL),
# endif
# ifdef CACHEFLUSH_D_WB
	XLAT(CACHEFLUSH_D_WB),
# endif
# ifdef CACHEFLUSH_D_PURGE
	XLAT(CACHEFLUSH_D_PURGE),
# endif
# ifdef CACHEFLUSH_I
	XLAT(CACHEFLUSH_I),
# endif
	XLAT_END
};

SYS_FUNC(cacheflush)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	/* len */
	tprintf(", %lu, ", tcp->u_arg[1]);
	/* flags */
	printflags(cacheflush_flags, tcp->u_arg[2], "CACHEFLUSH_???");

	return RVAL_DECODED;
}
#endif /* SH */

#ifdef NIOS2
SYS_FUNC(cacheflush)
{
	/* addr */
	printaddr(tcp->u_arg[0]);
	/* len */
	tprintf(", %lu, ", tcp->u_arg[3]);
	/* scope and flags (cache type) are currently ignored */

	return RVAL_DECODED;
}
#endif /* NIOS2 */
