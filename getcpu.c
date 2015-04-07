#include "defs.h"

SYS_FUNC(getcpu)
{
	if (exiting(tcp)) {
		unsigned u;
		if (tcp->u_arg[0] == 0)
			tprints("NULL, ");
		else if (umove(tcp, tcp->u_arg[0], &u) < 0)
			tprintf("%#lx, ", tcp->u_arg[0]);
		else
			tprintf("[%u], ", u);
		if (tcp->u_arg[1] == 0)
			tprints("NULL, ");
		else if (umove(tcp, tcp->u_arg[1], &u) < 0)
			tprintf("%#lx, ", tcp->u_arg[1]);
		else
			tprintf("[%u], ", u);
		tprintf("%#lx", tcp->u_arg[2]);
	}
	return 0;
}
