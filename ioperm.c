#include "defs.h"

SYS_FUNC(ioperm)
{
	tprintf("%#" PRI_klx ", %#" PRI_klx ", %d",
		tcp->u_arg[0], tcp->u_arg[1], (int) tcp->u_arg[2]);

	return RVAL_DECODED;
}
