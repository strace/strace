#include "defs.h"

SYS_FUNC(statfs64)
{
	const unsigned long size = tcp->u_arg[1];

	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu, ", size);
	} else {
		print_struct_statfs64(tcp, tcp->u_arg[2], size);
	}
	return 0;
}
