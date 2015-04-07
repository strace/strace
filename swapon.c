#include "defs.h"

#include <sys/swap.h>

#ifndef SWAP_FLAG_PREFER
# define SWAP_FLAG_PREFER 0x8000
#endif
#ifndef SWAP_FLAG_DISCARD
# define SWAP_FLAG_DISCARD 0x10000
#endif
#ifndef SWAP_FLAG_DISCARD_ONCE
# define SWAP_FLAG_DISCARD_ONCE 0x20000
#endif
#ifndef SWAP_FLAG_DISCARD_PAGES
# define SWAP_FLAG_DISCARD_PAGES 0x40000
#endif

#include "xlat/swap_flags.h"

SYS_FUNC(swapon)
{
	if (entering(tcp)) {
		int flags = tcp->u_arg[1];
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printflags(swap_flags, flags & ~SWAP_FLAG_PRIO_MASK,
			"SWAP_FLAG_???");
		if (flags & SWAP_FLAG_PREFER)
			tprintf("|%d", flags & SWAP_FLAG_PRIO_MASK);
	}
	return 0;
}
