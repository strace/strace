/*
 * Copyright (c) 2019-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/watchdog.h>

#include "xlat/watchdog_ioctl_flags.h"
#include "xlat/watchdog_ioctl_setoptions.h"

#define XLAT_MACROS_ONLY
#include "xlat/watchdog_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

int
watchdog_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	struct watchdog_info ident;
	unsigned int options;

	switch (code) {
	case WDIOC_GETSUPPORT:
		if (entering(tcp)) {
			tprint_arg_next();
			return 0;
		}
		if (umove_or_printaddr(tcp, arg, &ident))
			break;
		tprint_struct_begin();
		PRINT_FIELD_FLAGS(ident, options, watchdog_ioctl_flags,
				  "WDIOF_???");
		tprint_struct_next();
		if (abbrev(tcp)) {
			tprint_more_data_follows();
		} else {
			PRINT_FIELD_X(ident, firmware_version);
			tprint_struct_next();
			PRINT_FIELD_CSTRING(ident, identity);
		}
		tprint_struct_end();
		break;
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
	case WDIOC_SETOPTIONS:
		tprint_arg_next();
		if (umove_or_printaddr(tcp, arg, &options))
			break;
		tprint_indirect_begin();
		printflags(watchdog_ioctl_setoptions, options,
			   "WDIOS_???");
		tprint_indirect_end();
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
