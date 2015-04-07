#include "defs.h"

#ifdef HAVE_ASM_CACHECTL_H
# include <asm/cachectl.h>
#endif

#ifdef M68K
# include "xlat/cacheflush_scope.h"

static const struct xlat cacheflush_flags[] = {
#ifdef FLUSH_CACHE_BOTH
	XLAT(FLUSH_CACHE_BOTH),
#endif
#ifdef FLUSH_CACHE_DATA
	XLAT(FLUSH_CACHE_DATA),
#endif
#ifdef FLUSH_CACHE_INSN
	XLAT(FLUSH_CACHE_INSN),
#endif
	XLAT_END
};

SYS_FUNC(cacheflush)
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* scope */
		printxval(cacheflush_scope, tcp->u_arg[1], "FLUSH_SCOPE_???");
		tprints(", ");
		/* flags */
		printflags(cacheflush_flags, tcp->u_arg[2], "FLUSH_CACHE_???");
		/* len */
		tprintf(", %lu", tcp->u_arg[3]);
	}
	return 0;
}
#endif /* M68K */

#ifdef BFIN
static const struct xlat cacheflush_flags[] = {
	XLAT(ICACHE),
	XLAT(DCACHE),
	XLAT(BCACHE),
	XLAT_END
};

SYS_FUNC(cacheflush)
{
	if (entering(tcp)) {
		/* start addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* length */
		tprintf("%ld, ", tcp->u_arg[1]);
		/* flags */
		printxval(cacheflush_flags, tcp->u_arg[1], "?CACHE");
	}
	return 0;
}
#endif /* BFIN */

#ifdef SH
static const struct xlat cacheflush_flags[] = {
#ifdef CACHEFLUSH_D_INVAL
	XLAT(CACHEFLUSH_D_INVAL),
#endif
#ifdef CACHEFLUSH_D_WB
	XLAT(CACHEFLUSH_D_WB),
#endif
#ifdef CACHEFLUSH_D_PURGE
	XLAT(CACHEFLUSH_D_PURGE),
#endif
#ifdef CACHEFLUSH_I
	XLAT(CACHEFLUSH_I),
#endif
	XLAT_END
};

SYS_FUNC(cacheflush)
{
	if (entering(tcp)) {
		/* addr */
		tprintf("%#lx, ", tcp->u_arg[0]);
		/* len */
		tprintf("%lu, ", tcp->u_arg[1]);
		/* flags */
		printflags(cacheflush_flags, tcp->u_arg[2], "CACHEFLUSH_???");
	}
	return 0;
}
#endif /* SH */
