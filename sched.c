#include "defs.h"

#include <sched.h>

#include "xlat/schedulers.h"

SYS_FUNC(sched_getscheduler)
{
	if (entering(tcp)) {
		tprintf("%d", (int) tcp->u_arg[0]);
	} else if (!syserror(tcp)) {
		tcp->auxstr = xlookup(schedulers, tcp->u_rval);
		if (tcp->auxstr != NULL)
			return RVAL_STR;
	}
	return 0;
}

SYS_FUNC(sched_setscheduler)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printxval(schedulers, tcp->u_arg[1], "SCHED_???");
		tprints(", ");
		printnum_int(tcp, tcp->u_arg[2], "%d");
	}
	return 0;
}

SYS_FUNC(sched_getparam)
{
	if (entering(tcp))
		tprintf("%d, ", (int) tcp->u_arg[0]);
	else
		printnum_int(tcp, tcp->u_arg[1], "%d");
	return 0;
}

SYS_FUNC(sched_setparam)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printnum_int(tcp, tcp->u_arg[1], "%d");
	}
	return 0;
}

SYS_FUNC(sched_get_priority_min)
{
	if (entering(tcp)) {
		printxval(schedulers, tcp->u_arg[0], "SCHED_???");
	}
	return 0;
}

SYS_FUNC(sched_rr_get_interval)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			printaddr(tcp->u_arg[1]);
		else
			print_timespec(tcp, tcp->u_arg[1]);
	}
	return 0;
}
