#include "defs.h"

#ifdef HAVE_LINUX_FALLOC_H
# include <linux/falloc.h>
#endif

#include "xlat/falloc_flags.h"

SYS_FUNC(fallocate)
{
	if (entering(tcp)) {
		int argn;

		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");

		/* mode */
		printflags(falloc_flags, tcp->u_arg[1], "FALLOC_FL_???");
		tprints(", ");

		/* offset */
		argn = printllval(tcp, "%llu, ", 2);

		/* len */
		printllval(tcp, "%llu", argn);
	}
	return 0;
}
