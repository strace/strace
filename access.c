#include "defs.h"

#include <fcntl.h>

#include "xlat/access_flags.h"

static int
decode_access(struct tcb *tcp, int offset)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[offset]);
		tprints(", ");
		printflags(access_flags, tcp->u_arg[offset + 1], "?_OK");
	}
	return 0;
}

SYS_FUNC(access)
{
	return decode_access(tcp, 0);
}

SYS_FUNC(faccessat)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_access(tcp, 1);
}
