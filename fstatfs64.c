#include "defs.h"

SYS_FUNC(fstatfs64)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %" PRI_klu ", ", tcp->u_arg[1]);
	} else {
		print_struct_statfs64(tcp, tcp->u_arg[2], tcp->u_arg[1]);
	}
	return 0;
}
