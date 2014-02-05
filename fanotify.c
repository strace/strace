#include "defs.h"
#include <linux/fanotify.h>

static const struct xlat fan_classes[] = {
	XLAT(FAN_CLASS_NOTIF),
	XLAT(FAN_CLASS_CONTENT),
	XLAT(FAN_CLASS_PRE_CONTENT),
	XLAT_END
};

static const struct xlat fan_init_flags[] = {
	XLAT(FAN_CLOEXEC),
	XLAT(FAN_NONBLOCK),
	XLAT(FAN_UNLIMITED_QUEUE),
	XLAT(FAN_UNLIMITED_MARKS),
	XLAT_END
};

int
sys_fanotify_init(struct tcb *tcp)
{
	unsigned flags;

	if (exiting(tcp))
		return 0;

	flags = tcp->u_arg[0];
	printxval(fan_classes, flags & FAN_ALL_CLASS_BITS, "FAN_CLASS_???");
	flags &= ~FAN_ALL_CLASS_BITS;
	if (flags) {
		tprints("|");
		printflags(fan_init_flags, flags, "FAN_???");
	}
	tprints(", ");
	tprint_open_modes((unsigned) tcp->u_arg[1]);

	return 0;
}

static const struct xlat fan_mark_flags[] = {
	XLAT(FAN_MARK_ADD),
	XLAT(FAN_MARK_REMOVE),
	XLAT(FAN_MARK_DONT_FOLLOW),
	XLAT(FAN_MARK_ONLYDIR),
	XLAT(FAN_MARK_MOUNT),
	XLAT(FAN_MARK_IGNORED_MASK),
	XLAT(FAN_MARK_IGNORED_SURV_MODIFY),
	XLAT(FAN_MARK_FLUSH),
	XLAT_END
};

static const struct xlat fan_event_flags[] = {
	XLAT(FAN_ACCESS),
	XLAT(FAN_MODIFY),
	XLAT(FAN_CLOSE),
	XLAT(FAN_CLOSE_WRITE),
	XLAT(FAN_CLOSE_NOWRITE),
	XLAT(FAN_OPEN),
	XLAT(FAN_Q_OVERFLOW),
	XLAT(FAN_OPEN_PERM),
	XLAT(FAN_ACCESS_PERM),
	XLAT(FAN_ONDIR),
	XLAT(FAN_EVENT_ON_CHILD),
	XLAT_END
};

int
sys_fanotify_mark(struct tcb *tcp)
{
	if (exiting(tcp))
		return 0;

	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(fan_mark_flags, (unsigned) tcp->u_arg[1], "FAN_MARK_???");
	tprints(", ");
	printflags(fan_event_flags, tcp->u_arg[2], "FAN_???");
	tprints(", ");
	if ((int) tcp->u_arg[3] == FAN_NOFD)
		tprints("FAN_NOFD, ");
	else
		print_dirfd(tcp, tcp->u_arg[3]);
	printpath(tcp, tcp->u_arg[4]);

	return 0;
}
