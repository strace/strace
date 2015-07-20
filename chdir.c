#include "defs.h"

SYS_FUNC(chdir)
{
	printpath(tcp, tcp->u_arg[0]);

	return RVAL_DECODED;
}
