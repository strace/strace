#include "defs.h"
#include <fcntl.h>
#include <linux/inotify.h>

static const struct xlat inotify_flags[] = {
	XLAT(IN_ACCESS),
	XLAT(IN_MODIFY),
	XLAT(IN_ATTRIB),
	XLAT(IN_CLOSE),
	XLAT(IN_CLOSE_WRITE),
	XLAT(IN_CLOSE_NOWRITE),
	XLAT(IN_OPEN),
	XLAT(IN_MOVE),
	XLAT(IN_MOVED_FROM),
	XLAT(IN_MOVED_TO),
	XLAT(IN_CREATE),
	XLAT(IN_DELETE),
	XLAT(IN_DELETE_SELF),
	XLAT(IN_MOVE_SELF),
	XLAT(IN_UNMOUNT),
	XLAT(IN_Q_OVERFLOW),
	XLAT(IN_IGNORED),
	XLAT(IN_ONLYDIR),
	XLAT(IN_DONT_FOLLOW),
	XLAT(IN_EXCL_UNLINK),
	XLAT(IN_MASK_ADD),
	XLAT(IN_ISDIR),
	XLAT(IN_ONESHOT),
	XLAT_END
};

static const struct xlat inotify_init_flags[] = {
	XLAT(O_NONBLOCK),
	XLAT(O_CLOEXEC),
	XLAT_END
};

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
