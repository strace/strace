#include "defs.h"

SYS_FUNC(get_robust_list)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	} else {
		printnum_ptr(tcp, tcp->u_arg[1]);
		tprints(", ");
		printnum_ulong(tcp, tcp->u_arg[2]);
	}
	return 0;
}
