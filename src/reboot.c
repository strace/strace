/*
 * Copyright (c) 2014-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/bootflags1.h"
#include "xlat/bootflags2.h"
#include "xlat/bootflags3.h"

SYS_FUNC(reboot)
{
	const unsigned int magic1 = tcp->u_arg[0];
	const unsigned int magic2 = tcp->u_arg[1];
	const unsigned int cmd = tcp->u_arg[2];

	/* magic */
	tprints_arg_name("magic");
	printxval(bootflags1, magic1, "LINUX_REBOOT_MAGIC_???");

	/* magic2 */
	tprints_arg_next_name("magic2");
	printxval(bootflags2, magic2, "LINUX_REBOOT_MAGIC_???");

	/* cmd */
	tprints_arg_next_name("op");
	printxval(bootflags3, cmd, "LINUX_REBOOT_CMD_???");
	if (cmd == LINUX_REBOOT_CMD_RESTART2) {
		/*
		 * The size of kernel buffer is 256 bytes and
		 * the last byte is always zero, at most 255 bytes
		 * are copied from the user space.
		 */
		/* arg */
		tprints_arg_next_name("arg");
		printstr_ex(tcp, tcp->u_arg[3], 255, QUOTE_0_TERMINATED);
	}
	return RVAL_DECODED;
}
