#include "defs.h"

SYS_FUNC(chdir)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
	}
	return 0;
}
