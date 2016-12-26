#include "defs.h"

SYS_FUNC(truncate)
{
	printpath(tcp, tcp->u_arg[0]);
	tprintf(", %" PRI_klu, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(truncate64)
{
	printpath(tcp, tcp->u_arg[0]);
	printllval(tcp, ", %llu", 1);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate)
{
	printfd(tcp, tcp->u_arg[0]);
	tprintf(", %" PRI_klu, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(ftruncate64)
{
	printfd(tcp, tcp->u_arg[0]);
	printllval(tcp, ", %llu", 1);

	return RVAL_DECODED;
}
