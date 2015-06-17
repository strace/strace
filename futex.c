#include "defs.h"

#ifdef HAVE_LINUX_FUTEX_H
# include <linux/futex.h>
#endif

#ifndef FUTEX_PRIVATE_FLAG
# define FUTEX_PRIVATE_FLAG 128
#endif
#ifndef FUTEX_CLOCK_REALTIME
# define FUTEX_CLOCK_REALTIME 256
#endif

#include "xlat/futexops.h"
#include "xlat/futexwakeops.h"
#include "xlat/futexwakecmps.h"

SYS_FUNC(futex)
{
	if (entering(tcp)) {
		long int cmd = tcp->u_arg[1] & 127;
		tprintf("%p, ", (void *) tcp->u_arg[0]);
		printxval(futexops, tcp->u_arg[1], "FUTEX_???");
		tprintf(", %ld", tcp->u_arg[2]);
		if (cmd == FUTEX_WAKE_BITSET)
			tprintf(", %lx", tcp->u_arg[5]);
		else if (cmd == FUTEX_WAIT) {
			tprints(", ");
			printtv(tcp, tcp->u_arg[3]);
		} else if (cmd == FUTEX_WAIT_BITSET) {
			tprints(", ");
			printtv(tcp, tcp->u_arg[3]);
			tprintf(", %lx", tcp->u_arg[5]);
		} else if (cmd == FUTEX_REQUEUE)
			tprintf(", %ld, %p", tcp->u_arg[3], (void *) tcp->u_arg[4]);
		else if (cmd == FUTEX_CMP_REQUEUE || cmd == FUTEX_CMP_REQUEUE_PI)
			tprintf(", %ld, %p, %ld", tcp->u_arg[3], (void *) tcp->u_arg[4], tcp->u_arg[5]);
		else if (cmd == FUTEX_WAKE_OP) {
			tprintf(", %ld, %p, {", tcp->u_arg[3], (void *) tcp->u_arg[4]);
			if ((tcp->u_arg[5] >> 28) & 8)
				tprints("FUTEX_OP_OPARG_SHIFT|");
			printxval(futexwakeops, (tcp->u_arg[5] >> 28) & 0x7, "FUTEX_OP_???");
			tprintf(", %ld, ", (tcp->u_arg[5] >> 12) & 0xfff);
			if ((tcp->u_arg[5] >> 24) & 8)
				tprints("FUTEX_OP_OPARG_SHIFT|");
			printxval(futexwakecmps, (tcp->u_arg[5] >> 24) & 0x7, "FUTEX_OP_CMP_???");
			tprintf(", %ld}", tcp->u_arg[5] & 0xfff);
		} else if (cmd == FUTEX_WAIT_REQUEUE_PI) {
			tprints(", ");
			printtv(tcp, tcp->u_arg[3]);
			tprintf(", %p", (void *) tcp->u_arg[4]);
		}
	}
	return 0;
}
