#include "defs.h"

#ifdef HAVE_LINUX_PERF_EVENT_H
# include <linux/perf_event.h>
#endif

#include "xlat/perf_event_open_flags.h"

SYS_FUNC(perf_event_open)
{
	printaddr(tcp->u_arg[0]);
	tprintf(", %d, %d, %d, ",
		(int) tcp->u_arg[1],
		(int) tcp->u_arg[2],
		(int) tcp->u_arg[3]);
	printflags(perf_event_open_flags, tcp->u_arg[4], "PERF_FLAG_???");

	return RVAL_DECODED | RVAL_FD;
}
