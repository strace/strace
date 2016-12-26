#include "defs.h"

SYS_FUNC(readahead)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld", 1);
	tprintf(", %" PRI_klu, tcp->u_arg[argn]);

	return RVAL_DECODED;
}
