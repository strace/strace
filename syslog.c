/*
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/syslog_action_type.h"

SYS_FUNC(syslog)
{
	int type = tcp->u_arg[0];

	if (entering(tcp)) {
		/* type */
		printxval_ex(syslog_action_type, type, "SYSLOG_ACTION_???",
			     XLAT_STYLE_VERBOSE | XLAT_STYLE_FMT_D);
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
		printaddr(tcp->u_arg[1]);
		tprintf(", %" PRI_klu, tcp->u_arg[2]);
		return RVAL_DECODED;
	}

	/* bufp */
	if (syserror(tcp))
		printaddr(tcp->u_arg[1]);
	else
		printstrn(tcp, tcp->u_arg[1], tcp->u_rval);
	/* len */
	tprintf(", %d", (int) tcp->u_arg[2]);

	return 0;
}
