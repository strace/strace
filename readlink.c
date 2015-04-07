#include "defs.h"

static int
decode_readlink(struct tcb *tcp, int offset)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[offset]);
		tprints(", ");
	} else {
		if (syserror(tcp))
			tprintf("%#lx", tcp->u_arg[offset + 1]);
		else
			/* Used to use printpathn(), but readlink
			 * neither includes NUL in the returned count,
			 * nor actually writes it into memory.
			 * printpathn() would decide on printing
			 * "..." continuation based on garbage
			 * past return buffer's end.
			 */
			printstr(tcp, tcp->u_arg[offset + 1], tcp->u_rval);
		tprintf(", %lu", tcp->u_arg[offset + 2]);
	}
	return 0;
}

SYS_FUNC(readlink)
{
	return decode_readlink(tcp, 0);
}

SYS_FUNC(readlinkat)
{
	if (entering(tcp))
		print_dirfd(tcp, tcp->u_arg[0]);
	return decode_readlink(tcp, 1);
}
