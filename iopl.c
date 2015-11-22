#include "defs.h"

SYS_FUNC(iopl)
{
	tprintf("%d", (int) tcp->u_arg[0]);

	return RVAL_DECODED;
}
