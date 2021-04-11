/*
 * Copyright (c) 2019-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>
#include <linux/mount.h>
#include "xlat/mount_setattr_flags.h"
#include "xlat/mount_attr_attr.h"
#include "xlat/mount_attr_propagation.h"

static void
print_mount_attr(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const kernel_ulong_t size)
{
	struct mount_attr attr;

	if (size < MOUNT_ATTR_SIZE_VER0) {
		printaddr(addr);
		return;
	}

	if (umoven_or_printaddr(tcp, addr, MIN(sizeof(attr), size), &attr))
		return;

	tprint_struct_begin();

	PRINT_FIELD_FLAGS(attr, attr_set, mount_attr_attr, "MOUNT_ATTR_???");
	tprint_struct_next();

	PRINT_FIELD_FLAGS(attr, attr_clr, mount_attr_attr, "MOUNT_ATTR_???");
	tprint_struct_next();

	PRINT_FIELD_XVAL(attr, propagation, mount_attr_propagation, "MS_???");
	tprint_struct_next();

	if (attr.userns_fd > INT_MAX ||
	    !((attr.attr_set | attr.attr_clr) & MOUNT_ATTR_IDMAP)) {
		PRINT_FIELD_U(attr, userns_fd);
	} else {
		PRINT_FIELD_FD(attr, userns_fd, tcp);
	}

	if (size > sizeof(attr)) {
		print_nonzero_bytes(tcp, tprint_struct_next, addr, sizeof(attr),
				    MIN(size, get_pagesize()), QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

SYS_FUNC(mount_setattr)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* pathname */
	printpath(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* flags */
	printflags(mount_setattr_flags, tcp->u_arg[2], "AT_???");
	tprint_arg_next();

	/* uattr */
	print_mount_attr(tcp, tcp->u_arg[3], tcp->u_arg[4]);
	tprint_arg_next();

	/* usize */
	PRINT_VAL_U(tcp->u_arg[4]);

	return RVAL_DECODED | RVAL_FD;
}
