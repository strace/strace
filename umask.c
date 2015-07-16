#include "defs.h"

SYS_FUNC(umask)
{
	tprintf("%#lo", tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_OCTAL;
}
