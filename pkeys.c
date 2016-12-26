#include "defs.h"

#include "xlat/pkey_access.h"

SYS_FUNC(pkey_alloc)
{
	tprintf("%#" PRI_klx ", ", tcp->u_arg[0]);
	printflags64(pkey_access, tcp->u_arg[1], "PKEY_???");

	return RVAL_DECODED;
}

SYS_FUNC(pkey_free)
{
	tprintf("%d", (int) tcp->u_arg[0]);

	return RVAL_DECODED;
}
