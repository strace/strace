#include "defs.h"

SYS_FUNC(sethostname)
{
	printstr(tcp, tcp->u_arg[0], tcp->u_arg[1]);
	tprintf(", %lu", tcp->u_arg[1]);

	return RVAL_DECODED;
}

#if defined(ALPHA)
SYS_FUNC(gethostname)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			printaddr(tcp->u_arg[0]);
		else
			printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}
#endif /* ALPHA */
