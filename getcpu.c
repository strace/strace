#include "defs.h"

SYS_FUNC(getcpu)
{
	if (exiting(tcp)) {
		printnum_int(tcp, tcp->u_arg[0], "%u");
		tprints(", ");
		printnum_int(tcp, tcp->u_arg[1], "%u");
		tprints(", ");
		printaddr(tcp->u_arg[2]);
	}
	return 0;
}
