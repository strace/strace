#include "defs.h"

int
sys_truncate(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_truncate64(struct tcb *tcp)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		printllval(tcp, ", %llu", 1);
	}
	return 0;
}

int
sys_ftruncate(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %lu", tcp->u_arg[1]);
	}
	return 0;
}

int
sys_ftruncate64(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		printllval(tcp, ", %llu", 1);
	}
	return 0;
}
