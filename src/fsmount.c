/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/mount.h>
#include "xlat/fsmount_flags.h"
#include "xlat/fsmount_attr_flags.h"

SYS_FUNC(fsmount)
{
	/* fd */
	tprints_arg_name("fd");
	printfd(tcp, tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	printflags(fsmount_flags, tcp->u_arg[1], "FSMOUNT_???");

	/* attr_flags */
	tprints_arg_next_name("attr_flags");
	printflags(fsmount_attr_flags, tcp->u_arg[2], "MOUNT_ATTR_???");

	return RVAL_DECODED | RVAL_FD;
}
