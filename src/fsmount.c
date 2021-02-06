/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mount.h>
#include "xlat/fsmount_flags.h"
#include "xlat/mount_attr_flags.h"

SYS_FUNC(fsmount)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");

	printflags(fsmount_flags, tcp->u_arg[1], "FSMOUNT_???");
	tprints(", ");

	printflags(mount_attr_flags, tcp->u_arg[2], "MOUNT_ATTR_???");

	return RVAL_DECODED | RVAL_FD;
}
