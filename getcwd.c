#include "defs.h"

SYS_FUNC(getcwd)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			printaddr(tcp->u_arg[0]);
		else
			printpathn(tcp, tcp->u_arg[0], tcp->u_rval - 1);
		tprintf(", %" PRI_klu, tcp->u_arg[1]);
	}
	return 0;
}
