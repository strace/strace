#include "defs.h"
#include <fcntl.h>

#include "xlat/inotify_flags.h"
#include "xlat/inotify_init_flags.h"

SYS_FUNC(inotify_add_watch)
{
	/* file descriptor */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	/* pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprints(", ");
	/* mask */
	printflags(inotify_flags, tcp->u_arg[2], "IN_???");

	return RVAL_DECODED;
}

SYS_FUNC(inotify_rm_watch)
{
	/* file descriptor */
	printfd(tcp, tcp->u_arg[0]);
	/* watch descriptor */
	tprintf(", %d", (int) tcp->u_arg[1]);

	return RVAL_DECODED;
}

SYS_FUNC(inotify_init1)
{
	printflags(inotify_init_flags, tcp->u_arg[0], "IN_???");

	return RVAL_DECODED | RVAL_FD;
}
