#include "defs.h"

SYS_FUNC(getpagesize)
{
	return RVAL_DECODED | RVAL_HEX;
}
