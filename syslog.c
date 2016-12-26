/*
 * Copyright (c) 2012-2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
