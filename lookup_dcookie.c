#include "defs.h"

SYS_FUNC(lookup_dcookie)
{
	if (entering(tcp))
		return 0;

	/* cookie */
	int argn = printllval(tcp, "%llu", 0);
	tprints(", ");

	/* buffer */
	if (syserror(tcp))
		printaddr(tcp->u_arg[argn]);
	else
		printstr(tcp, tcp->u_arg[argn], tcp->u_rval);

	/* len */
	tprintf(", %lu", tcp->u_arg[argn + 1]);

	return 0;
}
