#include "defs.h"

int
sys_chdir(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
	}
	return 0;
}
