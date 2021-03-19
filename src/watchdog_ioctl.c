/*
 * Copyright (c) 2019-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/watchdog.h>

#define XLAT_MACROS_ONLY
#include "xlat/watchdog_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

int
watchdog_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case WDIOC_GETSTATUS:
	case WDIOC_GETBOOTSTATUS:
	case WDIOC_GETTEMP:
	case WDIOC_GETTIMEOUT:
	case WDIOC_GETPRETIMEOUT:
	case WDIOC_GETTIMELEFT:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case WDIOC_SETTIMEOUT:
	case WDIOC_SETPRETIMEOUT:
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	/*
	 * linux/watchdog.h says that this takes an int, but in
	 * practice the argument is ignored.
	 */
	case WDIOC_KEEPALIVE:
		break;
	default:
		return RVAL_DECODED;
	}
	return RVAL_IOCTL_DECODED;
}
