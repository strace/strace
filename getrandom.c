#include "defs.h"
#include "xlat/getrandom_flags.h"

SYS_FUNC(getrandom)
{
	if (exiting(tcp)) {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[0]);
		else
			printstr(tcp, tcp->u_arg[0], tcp->u_rval);
		tprintf(", %lu, ", tcp->u_arg[1]);
		printflags(getrandom_flags, tcp->u_arg[2], "GRND_???");
	}
	return 0;
}
