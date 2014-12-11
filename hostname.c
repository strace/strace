#include "defs.h"

int
sys_sethostname(struct tcb *tcp)
{
	if (entering(tcp)) {
		printstr(tcp, tcp->u_arg[0], tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

#if defined(ALPHA)
int
sys_gethostname(struct tcb *tcp)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printstr(tcp, tcp->u_arg[0], -1);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}
#endif /* ALPHA */
