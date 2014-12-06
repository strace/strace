#include "defs.h"

int
sys_umask(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf("%#lo", tcp->u_arg[0]);
	}
	return RVAL_OCTAL;
}
