#include "defs.h"

SYS_FUNC(sethostname)
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

#if defined(ALPHA)
SYS_FUNC(gethostname)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}
#endif /* ALPHA */
