#include "defs.h"

SYS_FUNC(umask)
{
	if (entering(tcp)) {
		tprintf("%#lo", tcp->u_arg[0]);
	}
	return RVAL_OCTAL;
}
