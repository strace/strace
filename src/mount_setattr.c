/*
 * Copyright (c) 2019-2025 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_fcntl.h"
#include <linux/fcntl.h>
#include <linux/mount.h>
#include "xlat/mount_setattr_flags.h"
#include "xlat/mount_attr_attr.h"
#include "xlat/mount_attr_propagation.h"
#include "xlat/open_tree_flags.h"

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

static void
decode_dfd_file_flags(struct tcb *const tcp,
		      const int dirfd,
		      const kernel_ulong_t fname,
		      const struct xlat *const x,
		      const unsigned int flags,
		      const char *const dflt)
{
	tprints_arg_name("dirfd");
	print_dirfd(tcp, dirfd);

	tprints_arg_next_name("pathname");
	printpath(tcp, fname);

	tprints_arg_next_name("flags");
	printflags(x, flags, dflt);

}

static void
decode_dfd_file_flags_attr(struct tcb *const tcp,
			   const int dirfd,
			   const kernel_ulong_t fname,
			   const struct xlat *const x,
			   const unsigned int flags,
			   const char *const dflt,
			   const kernel_ulong_t attr,
			   const kernel_ulong_t attr_size)
{
	decode_dfd_file_flags(tcp, dirfd, fname, x, flags, dflt);

	tprints_arg_next_name("attr");
	print_mount_attr(tcp, attr, attr_size);

	tprints_arg_next_name("size");
	PRINT_VAL_U(attr_size);
}

SYS_FUNC(mount_setattr)
{
	decode_dfd_file_flags_attr(tcp,
				   tcp->u_arg[0],
				   tcp->u_arg[1],
				   mount_setattr_flags,
				   tcp->u_arg[2],
				   "AT_???",
				   tcp->u_arg[3],
				   tcp->u_arg[4]);

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(open_tree)
{
	decode_dfd_file_flags(tcp,
			      tcp->u_arg[0],
			      tcp->u_arg[1],
			      open_tree_flags,
			      tcp->u_arg[2],
			      "OPEN_TREE_???");

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(open_tree_attr)
{
	decode_dfd_file_flags_attr(tcp,
				   tcp->u_arg[0],
				   tcp->u_arg[1],
				   open_tree_flags,
				   tcp->u_arg[2],
				   "OPEN_TREE_???",
				   tcp->u_arg[3],
				   tcp->u_arg[4]);

	return RVAL_DECODED | RVAL_FD;
}
