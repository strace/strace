#include "defs.h"
#include <linux/fanotify.h>

#include "xlat/fan_classes.h"
#include "xlat/fan_init_flags.h"

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

#include "xlat/fan_mark_flags.h"
#include "xlat/fan_event_flags.h"

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
