#include "defs.h"

#include "xlat/membarrier_cmds.h"

SYS_FUNC(membarrier)
{
	if (entering(tcp)) {
		int cmd = tcp->u_arg[0], flags = tcp->u_arg[1];

		printxval(membarrier_cmds, cmd, "MEMBARRIER_CMD_???");
		tprintf(", %d", flags);

		return cmd ? RVAL_DECODED : 0;
	}

	if (syserror(tcp) || !tcp->u_rval)
		return 0;

	tcp->auxstr = sprintflags("", membarrier_cmds, tcp->u_rval);
	return RVAL_HEX | RVAL_STR;
}
