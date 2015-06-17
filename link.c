#include "defs.h"

#include <fcntl.h>

#include "xlat/at_flags.h"

SYS_FUNC(link)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printpath(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(linkat)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
		print_dirfd(tcp, tcp->u_arg[2]);
		printpath(tcp, tcp->u_arg[3]);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[4], "AT_???");
	}
	return 0;
}

SYS_FUNC(unlinkat)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[2], "AT_???");
	}
	return 0;
}

SYS_FUNC(symlinkat)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		print_dirfd(tcp, tcp->u_arg[1]);
		printpath(tcp, tcp->u_arg[2]);
	}
	return 0;
}
