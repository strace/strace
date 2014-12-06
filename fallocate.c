#include "defs.h"

int
sys_fallocate(struct tcb *tcp)
{
	if (entering(tcp)) {
		int argn;
		printfd(tcp, tcp->u_arg[0]);		/* fd */
		tprintf(", %#lo, ", tcp->u_arg[1]);	/* mode */
		argn = printllval(tcp, "%llu, ", 2);	/* offset */
		printllval(tcp, "%llu", argn);		/* len */
	}
	return 0;
}
