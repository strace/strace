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
		struct sched_param p;
		tprintf("%d, ", (int) tcp->u_arg[0]);
		printxval(schedulers, tcp->u_arg[1], "SCHED_???");
		if (umove(tcp, tcp->u_arg[2], &p) < 0)
			tprintf(", %#lx", tcp->u_arg[2]);
		else
			tprintf(", { %d }", p.sched_priority);
	}
	return 0;
}

SYS_FUNC(sched_getparam)
{
	if (entering(tcp)) {
		tprintf("%d, ", (int) tcp->u_arg[0]);
	} else {
		struct sched_param p;
		if (umove(tcp, tcp->u_arg[1], &p) < 0)
			tprintf("%#lx", tcp->u_arg[1]);
		else
			tprintf("{ %d }", p.sched_priority);
	}
	return 0;
}

SYS_FUNC(sched_setparam)
{
	if (entering(tcp)) {
		struct sched_param p;
		if (umove(tcp, tcp->u_arg[1], &p) < 0)
			tprintf("%d, %#lx", (int) tcp->u_arg[0], tcp->u_arg[1]);
		else
			tprintf("%d, { %d }", (int) tcp->u_arg[0], p.sched_priority);
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
		tprintf("%ld, ", (long) (pid_t) tcp->u_arg[0]);
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[1]);
		else
			print_timespec(tcp, tcp->u_arg[1]);
	}
	return 0;
}
