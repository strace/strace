#include "defs.h"

#include "xlat/memfd_create_flags.h"

SYS_FUNC(memfd_create)
{
	printstr(tcp, tcp->u_arg[0], -1);
	tprints(", ");
	printflags(memfd_create_flags, tcp->u_arg[1], "MFD_???");

	return RVAL_DECODED | RVAL_FD;
}
