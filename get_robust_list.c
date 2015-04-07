#include "defs.h"

SYS_FUNC(get_robust_list)
{
	if (entering(tcp)) {
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	} else {
		void *addr;
		size_t len;

		if (syserror(tcp) ||
		    !tcp->u_arg[1] ||
		    umove(tcp, tcp->u_arg[1], &addr) < 0) {
			tprintf("%#lx, ", tcp->u_arg[1]);
		} else {
			tprintf("[%p], ", addr);
		}

		if (syserror(tcp) ||
		    !tcp->u_arg[2] ||
		    umove(tcp, tcp->u_arg[2], &len) < 0) {
			tprintf("%#lx", tcp->u_arg[2]);
		} else {
			tprintf("[%lu]", (unsigned long) len);
		}
	}
	return 0;
}
