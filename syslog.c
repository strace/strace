#include "defs.h"

enum {
	SYSLOG_ACTION_CLOSE = 0,
	SYSLOG_ACTION_OPEN,
	SYSLOG_ACTION_READ,
	SYSLOG_ACTION_READ_ALL,
	SYSLOG_ACTION_READ_CLEAR,
	SYSLOG_ACTION_CLEAR,
	SYSLOG_ACTION_CONSOLE_OFF,
	SYSLOG_ACTION_CONSOLE_ON,
	SYSLOG_ACTION_CONSOLE_LEVEL,
	SYSLOG_ACTION_SIZE_UNREAD,
	SYSLOG_ACTION_SIZE_BUFFER
};

#include "xlat/syslog_action_type.h"

SYS_FUNC(syslog)
{
	int type = tcp->u_arg[0];

	if (entering(tcp)) {
		/* type */
		printxval(syslog_action_type, type, "SYSLOG_ACTION_???");
		tprints(", ");
	}

	switch (type) {
		case SYSLOG_ACTION_READ:
		case SYSLOG_ACTION_READ_ALL:
		case SYSLOG_ACTION_READ_CLEAR:
			if (entering(tcp))
				return 0;
			break;
		default:
			if (entering(tcp)) {
				tprintf("%#lx, %lu",
					tcp->u_arg[1], tcp->u_arg[2]);
			}
			return 0;
	}

	/* bufp */
	if (syserror(tcp))
		tprintf("%#lx", tcp->u_arg[1]);
	else
		printstr(tcp, tcp->u_arg[1], tcp->u_rval);
	/* len */
	tprintf(", %d", (int) tcp->u_arg[2]);

	return 0;
}
