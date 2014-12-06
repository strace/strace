#include "defs.h"

static int
decode_chmod(struct tcb *tcp, int offset)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[offset]);
		tprintf(", %#lo", tcp->u_arg[offset + 1]);
	}
	return 0;
}

int
sys_chmod(struct tcb *tcp)
{
	return decode_chmod(tcp, 0);
}

int
sys_fchmodat(struct tcb *tcp)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_chmod(tcp, 1);
}

int
sys_fchmod(struct tcb *tcp)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return 0;
}
