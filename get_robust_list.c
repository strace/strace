#include "defs.h"

SYS_FUNC(get_robust_list)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	} else {
		printnum_long(tcp, tcp->u_arg[1], "%#lx");
		tprints(", ");
		printnum_long(tcp, tcp->u_arg[2], "%lu");
	}
	return 0;
}
