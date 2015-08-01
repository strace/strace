#include "defs.h"

static int
do_eventfd(struct tcb *tcp, int flags_arg)
{
	tprintf("%lu", tcp->u_arg[0]);
	if (flags_arg >= 0) {
		tprints(", ");
		printflags(open_mode_flags, tcp->u_arg[flags_arg], "O_???");
	}

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(eventfd)
{
	return do_eventfd(tcp, -1);
}

SYS_FUNC(eventfd2)
{
	return do_eventfd(tcp, 1);
}
