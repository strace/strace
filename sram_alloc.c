#include "defs.h"

#ifdef BFIN

#include <bfin_sram.h>

#include "xlat/sram_alloc_flags.h"

SYS_FUNC(sram_alloc)
{
	if (entering(tcp)) {
		/* size */
		tprintf("%lu, ", tcp->u_arg[0]);
		/* flags */
		printflags(sram_alloc_flags, tcp->u_arg[1], "???_SRAM");
	}
	return 1;
}

#endif /* BFIN */
