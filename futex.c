#include "defs.h"

#ifdef HAVE_LINUX_FUTEX_H
# include <linux/futex.h>
#endif

#ifndef FUTEX_WAIT
# define FUTEX_WAIT 0
#endif
#ifndef FUTEX_WAKE
# define FUTEX_WAKE 1
#endif
#ifndef FUTEX_FD
# define FUTEX_FD 2
#endif
#ifndef FUTEX_REQUEUE
# define FUTEX_REQUEUE 3
#endif
#ifndef FUTEX_CMP_REQUEUE
# define FUTEX_CMP_REQUEUE 4
#endif
#ifndef FUTEX_WAKE_OP
# define FUTEX_WAKE_OP 5
#endif
#ifndef FUTEX_LOCK_PI
# define FUTEX_LOCK_PI 6
# define FUTEX_UNLOCK_PI 7
# define FUTEX_TRYLOCK_PI 8
#endif
#ifndef FUTEX_WAIT_BITSET
# define FUTEX_WAIT_BITSET 9
#endif
#ifndef FUTEX_WAKE_BITSET
# define FUTEX_WAKE_BITSET 10
#endif
#ifndef FUTEX_WAIT_REQUEUE_PI
# define FUTEX_WAIT_REQUEUE_PI 11
#endif
#ifndef FUTEX_CMP_REQUEUE_PI
# define FUTEX_CMP_REQUEUE_PI 12
#endif
#ifndef FUTEX_PRIVATE_FLAG
# define FUTEX_PRIVATE_FLAG 128
#endif
#ifndef FUTEX_CLOCK_REALTIME
# define FUTEX_CLOCK_REALTIME 256
#endif
#ifndef FUTEX_WAIT_PRIVATE
# define FUTEX_WAIT_PRIVATE		(FUTEX_WAIT | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_WAKE_PRIVATE
# define FUTEX_WAKE_PRIVATE		(FUTEX_WAKE | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_REQUEUE_PRIVATE
# define FUTEX_REQUEUE_PRIVATE		(FUTEX_REQUEUE | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_CMP_REQUEUE_PRIVATE
# define FUTEX_CMP_REQUEUE_PRIVATE 	(FUTEX_CMP_REQUEUE | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_WAKE_OP_PRIVATE
# define FUTEX_WAKE_OP_PRIVATE		(FUTEX_WAKE_OP | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_LOCK_PI_PRIVATE
# define FUTEX_LOCK_PI_PRIVATE		(FUTEX_LOCK_PI | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_UNLOCK_PI_PRIVATE
# define FUTEX_UNLOCK_PI_PRIVATE	(FUTEX_UNLOCK_PI | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_TRYLOCK_PI_PRIVATE
# define FUTEX_TRYLOCK_PI_PRIVATE 	(FUTEX_TRYLOCK_PI | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_WAIT_BITSET_PRIVATE
# define FUTEX_WAIT_BITSET_PRIVATE	(FUTEX_WAIT_BITSET | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_WAKE_BITSET_PRIVATE
# define FUTEX_WAKE_BITSET_PRIVATE	(FUTEX_WAKE_BITSET | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_WAIT_REQUEUE_PI_PRIVATE
# define FUTEX_WAIT_REQUEUE_PI_PRIVATE	(FUTEX_WAIT_REQUEUE_PI | FUTEX_PRIVATE_FLAG)
#endif
#ifndef FUTEX_CMP_REQUEUE_PI_PRIVATE
# define FUTEX_CMP_REQUEUE_PI_PRIVATE	(FUTEX_CMP_REQUEUE_PI | FUTEX_PRIVATE_FLAG)
#endif
#include "xlat/futexops.h"
#ifndef FUTEX_OP_SET
# define FUTEX_OP_SET		0
# define FUTEX_OP_ADD		1
# define FUTEX_OP_OR		2
# define FUTEX_OP_ANDN		3
# define FUTEX_OP_XOR		4
# define FUTEX_OP_CMP_EQ	0
# define FUTEX_OP_CMP_NE	1
# define FUTEX_OP_CMP_LT	2
# define FUTEX_OP_CMP_LE	3
# define FUTEX_OP_CMP_GT	4
# define FUTEX_OP_CMP_GE	5
#endif
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
