/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#ifdef HAVE_LINUX_MOUNT_H
# include <linux/mount.h>
#endif
#include "xlat/fsopen_flags.h"

SYS_FUNC(fsopen)
{
	printstr(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(fsopen_flags, tcp->u_arg[1], "FSOPEN_???");
	return RVAL_DECODED | RVAL_FD;
}
