#include "defs.h"

SYS_FUNC(truncate)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(truncate64)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		printllval(tcp, ", %llu", 1);
	}
	return 0;
}

SYS_FUNC(ftruncate)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(ftruncate64)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		printllval(tcp, ", %llu", 1);
	}
	return 0;
}
