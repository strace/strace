#include "defs.h"

#include <fcntl.h>

#include "xlat/at_flags.h"

SYS_FUNC(link)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printpath(tcp, tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(linkat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	print_dirfd(tcp, tcp->u_arg[2]);
	printpath(tcp, tcp->u_arg[3]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[4], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(unlinkat)
{
	print_dirfd(tcp, tcp->u_arg[0]);
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	printflags(at_flags, tcp->u_arg[2], "AT_???");

	return RVAL_DECODED;
}

SYS_FUNC(symlinkat)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	print_dirfd(tcp, tcp->u_arg[1]);
	printpath(tcp, tcp->u_arg[2]);

	return RVAL_DECODED;
}
