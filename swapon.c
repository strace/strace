#include "defs.h"

#include <sys/swap.h>

#include "xlat/swap_flags.h"

SYS_FUNC(swapon)
{
	int flags = tcp->u_arg[1];

	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(swap_flags, flags & ~SWAP_FLAG_PRIO_MASK,
		"SWAP_FLAG_???");
	if (flags & SWAP_FLAG_PREFER)
		tprintf("|%d", flags & SWAP_FLAG_PRIO_MASK);

	return RVAL_DECODED;
}
