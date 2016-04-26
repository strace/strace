#include "defs.h"

SYS_FUNC(fstatfs64)
{
	const unsigned long size = tcp->u_arg[1];

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %lu, ", size);
	} else {
		print_struct_statfs64(tcp, tcp->u_arg[2], size);
	}
	return 0;
}
