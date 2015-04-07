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

SYS_FUNC(chmod)
{
	return decode_chmod(tcp, 0);
}

SYS_FUNC(fchmodat)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_chmod(tcp, 1);
}

SYS_FUNC(fchmod)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %#lo", tcp->u_arg[1]);
	}
	return 0;
}
