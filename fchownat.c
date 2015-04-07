#include "defs.h"

SYS_FUNC(fchownat)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		printpath(tcp, tcp->u_arg[1]);
		printuid(", ", tcp->u_arg[2]);
		printuid(", ", tcp->u_arg[3]);
		tprints(", ");
		printflags(at_flags, tcp->u_arg[4], "AT_???");
	}
	return 0;
}
