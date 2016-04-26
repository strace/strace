#include "defs.h"

SYS_FUNC(fstatfs)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_struct_statfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}
