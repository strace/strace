/*
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/syslog_action_type.h"
#include "xlat/syslog_console_levels.h"

SYS_FUNC(syslog)
{
	int type = tcp->u_arg[0];
	int len = tcp->u_arg[2];

	if (entering(tcp)) {
		/* type */
		printxval_ex(syslog_action_type, type, "SYSLOG_ACTION_???",
			     XLAT_STYLE_VERBOSE | XLAT_STYLE_FMT_D);
	}

	switch (type) {
	/* Those commands have bufp and len ignored */
	case SYSLOG_ACTION_CLOSE:
	case SYSLOG_ACTION_OPEN:
	case SYSLOG_ACTION_CLEAR:
	case SYSLOG_ACTION_CONSOLE_OFF:
	case SYSLOG_ACTION_CONSOLE_ON:
	case SYSLOG_ACTION_SIZE_UNREAD:
	case SYSLOG_ACTION_SIZE_BUFFER:
		return RVAL_DECODED;

	case SYSLOG_ACTION_READ:
	case SYSLOG_ACTION_READ_ALL:
	case SYSLOG_ACTION_READ_CLEAR:
		if (entering(tcp)) {
			tprints(", ");
			return 0;
		}
		break;

	case SYSLOG_ACTION_CONSOLE_LEVEL: /* Uses len */
		tprints(", ");
		printaddr64(tcp->u_arg[1]);
		tprints(", ");
		printxval_ex(syslog_console_levels, len, "LOGLEVEL_???",
			     XLAT_STYLE_VERBOSE | XLAT_STYLE_FMT_D);
		return RVAL_DECODED;

	default:
		tprints(", ");
		printaddr64(tcp->u_arg[1]);
		tprintf(", %d", len);
		return RVAL_DECODED;
	}

	/* syscall exit handler for SYSLOG_ACTION_READ* */

	/* bufp */
	if (syserror(tcp))
		printaddr64(tcp->u_arg[1]);
	else
		printstrn(tcp, tcp->u_arg[1], tcp->u_rval);
	/* len */
	tprintf(", %d", len);

	return 0;
}
