#include "defs.h"
#include <fcntl.h>
#include <linux/inotify.h>

#include "xlat/inotify_flags.h"
#include "xlat/inotify_init_flags.h"

int
sys_inotify_add_watch(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* file descriptor */
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		/* pathname */
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");
		/* mask */
		printflags(inotify_flags, tcp->u_arg[2], "IN_???");
	}
	return 0;
}

int
sys_inotify_rm_watch(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* file descriptor */
		printfd(tcp, tcp->u_arg[0]);
		/* watch descriptor */
		tprintf(", %d", (int) tcp->u_arg[1]);
	}
	return 0;
}

int
sys_inotify_init1(struct tcb *tcp)
{
	if (entering(tcp))
		printflags(inotify_init_flags, tcp->u_arg[0], "IN_???");
	return 0;
}
