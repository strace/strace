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
	const long uaddr = tcp->u_arg[0];
	const int op = tcp->u_arg[1];
	const int cmd = op & 127;
	const long timeout = tcp->u_arg[3];
	const long uaddr2 = tcp->u_arg[4];
	const unsigned int val = tcp->u_arg[2];
	const unsigned int val2 = tcp->u_arg[3];
	const unsigned int val3 = tcp->u_arg[5];

	printaddr(uaddr);
	tprints(", ");
	printxval(futexops, op, "FUTEX_???");
	tprintf(", %u", val);
	switch (cmd) {
	case FUTEX_WAIT:
	case FUTEX_LOCK_PI:
		tprints(", ");
		print_timespec(tcp, timeout);
		break;
	case FUTEX_WAIT_BITSET:
		tprints(", ");
		print_timespec(tcp, timeout);
		tprintf(", %x", val3);
		break;
	case FUTEX_WAKE_BITSET:
		tprintf(", %x", val3);
		break;
	case FUTEX_REQUEUE:
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		break;
	case FUTEX_CMP_REQUEUE:
	case FUTEX_CMP_REQUEUE_PI:
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		tprintf(", %u", val3);
		break;
	case FUTEX_WAKE_OP:
		tprintf(", %u, ", val2);
		printaddr(uaddr2);
		tprints(", {");
		if ((val3 >> 28) & 8)
			tprints("FUTEX_OP_OPARG_SHIFT|");
		printxval(futexwakeops, (val3 >> 28) & 0x7, "FUTEX_OP_???");
		tprintf(", %u, ", (val3 >> 12) & 0xfff);
		if ((val3 >> 24) & 8)
			tprints("FUTEX_OP_OPARG_SHIFT|");
		printxval(futexwakecmps, (val3 >> 24) & 0x7, "FUTEX_OP_CMP_???");
		tprintf(", %u}", val3 & 0xfff);
		break;
	case FUTEX_WAIT_REQUEUE_PI:
		tprints(", ");
		print_timespec(tcp, timeout);
		tprints(", ");
		printaddr(uaddr2);
		break;
	case FUTEX_WAKE:
	case FUTEX_UNLOCK_PI:
	case FUTEX_TRYLOCK_PI:
		break;
	default:
		tprintf(", %lx, %lx, %x", timeout, uaddr2, val3);
		break;
	}

	return RVAL_DECODED;
}
