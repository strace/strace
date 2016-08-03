#include "defs.h"

SYS_FUNC(umask)
{
	print_numeric_umode_t(tcp->u_arg[0]);

	return RVAL_DECODED | RVAL_OCTAL;
}
