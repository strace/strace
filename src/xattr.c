/*
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2025 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/fcntl.h>
#include <linux/xattr.h>

#include "xlat/xattrflags.h"
#include "xlat/xattrat_flags.h"

#ifndef XATTR_SIZE_MAX
# define XATTR_SIZE_MAX 65536
#endif

static void
print_xattr_val(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const kernel_ulong_t size)
{
	/* value */
	if (size > XATTR_SIZE_MAX)
		printaddr(addr);
	else
		printstr_ex(tcp, addr, size, QUOTE_OMIT_TRAILING_0);
}

static int
decode_setxattr_without_path(struct tcb *const tcp)
{
	/* name */
	tprint_arg_next();
	printstr(tcp, tcp->u_arg[1]);

	/* value */
	tprint_arg_next();
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3]);

	/* size */
	tprint_arg_next();
	PRINT_VAL_U(tcp->u_arg[3]);

	/* flags */
	tprint_arg_next();
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");

	return RVAL_DECODED;
}

SYS_FUNC(setxattr)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);

	return decode_setxattr_without_path(tcp);
}

SYS_FUNC(fsetxattr)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);

	return decode_setxattr_without_path(tcp);
}

static int
decode_getxattr_without_path(struct tcb *const tcp)
{
	if (entering(tcp)) {
		/* name */
		tprint_arg_next();
		printstr(tcp, tcp->u_arg[1]);

		if (tcp->u_arg[3])
			return 0;

		/* value */
		tprint_arg_next();
		printaddr(tcp->u_arg[2]);

		/* size */
		tprint_arg_next();
		PRINT_VAL_U(tcp->u_arg[3]);
	} else {
		/* value */
		tprint_arg_next();
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_rval);

		/* size */
		tprint_arg_next();
		PRINT_VAL_U(tcp->u_arg[3]);
	}

	return RVAL_DECODED;
}

SYS_FUNC(getxattr)
{
	if (entering(tcp)) {
		/* pathname */
		printpath(tcp, tcp->u_arg[0]);
	}

	return decode_getxattr_without_path(tcp);
}

SYS_FUNC(fgetxattr)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
	}

	return decode_getxattr_without_path(tcp);
}

static void
print_xattr_list(struct tcb *const tcp, const kernel_ulong_t addr,
		 const kernel_ulong_t size)
{
	/* list */
	tprint_arg_next();
	if (!size || syserror(tcp)) {
		printaddr(addr);
	} else {
		printstrn(tcp, addr, tcp->u_rval);
	}

	/* size */
	tprint_arg_next();
	PRINT_VAL_U(size);
}

SYS_FUNC(listxattr)
{
	if (entering(tcp)) {
		/* pathname */
		printpath(tcp, tcp->u_arg[0]);
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(flistxattr)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(removexattr)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);

	/* name */
	tprint_arg_next();
	printstr(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}

SYS_FUNC(fremovexattr)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);

	/* name */
	tprint_arg_next();
	printstr(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}

static void
decode_dirfd_pathname_flags(struct tcb *tcp)
{
	/* dirfd */
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprint_arg_next();
	printpath(tcp, tcp->u_arg[1]);

	/* flags */
	tprint_arg_next();
	printflags(xattrat_flags, tcp->u_arg[2], "AT_???");
}

static void
decode_dirfd_pathname_flags_name(struct tcb *tcp)
{
	/* dirfd, pathname, flags */
	decode_dirfd_pathname_flags(tcp);

	/* name */
	tprint_arg_next();
	printstr(tcp, tcp->u_arg[3]);
}

static int
umove_xattr_args_or_printaddr(struct tcb *tcp, struct xattr_args *args,
			      kernel_ulong_t addr, kernel_ulong_t size)
{
	enum { XATTR_ARGS_SIZE_VER0 = 16 };

	if (size < XATTR_ARGS_SIZE_VER0) {
		printaddr(addr);
		return -1;
	}
	if (umoven_or_printaddr(tcp, addr, MIN(size, sizeof(*args)), args))
		return -1;

	return 0;
}

static void
print_xattr_args(struct tcb *tcp, struct xattr_args *args,
		 kernel_ulong_t addr, kernel_ulong_t size,
		 bool decode_value)
{
	tprint_struct_begin();

	if (decode_value) {
		kernel_ulong_t val_size =
			entering(tcp) ? (kernel_long_t) args->size : tcp->u_rval;

		PRINT_FIELD_OBJ_TCB_VAL(*args, value, tcp, print_xattr_val,
					val_size);
	} else {
		PRINT_FIELD_ADDR64(*args, value);
	}

	tprint_struct_next();
	PRINT_FIELD_U(*args, size);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(*args, flags, xattrflags, "XATTR_???");

	if (size > sizeof(*args)) {
		print_nonzero_bytes(tcp, tprint_struct_next,
				    addr, sizeof(*args),
				    MIN(size, get_pagesize()), QUOTE_FORCE_HEX);
	}

	tprint_struct_end();
}

SYS_FUNC(setxattrat)
{
	const kernel_ulong_t addr = tcp->u_arg[4];
	const kernel_ulong_t size = tcp->u_arg[5];
	struct xattr_args args;

	/* dirfd, pathname, flags, name */
	decode_dirfd_pathname_flags_name(tcp);

	/* args */
	tprint_arg_next();
	if (!umove_xattr_args_or_printaddr(tcp, &args, addr, size))
		print_xattr_args(tcp, &args, addr, size, true);

	/* size */
	tprint_arg_next();
	PRINT_VAL_U(size);

	return RVAL_DECODED;
}

SYS_FUNC(getxattrat)
{
	const kernel_ulong_t addr = tcp->u_arg[4];
	const kernel_ulong_t size = tcp->u_arg[5];

	if (entering(tcp)) {
		struct xattr_args args;

		/* dirfd, pathname, flags, name */
		decode_dirfd_pathname_flags_name(tcp);

		/* args */
		tprint_arg_next();
		if (!umove_xattr_args_or_printaddr(tcp, &args, addr, size)) {
			if (args.size) {
				set_tcb_priv_data(tcp, xobjdup(&args), free);
				return 0;
			}
			print_xattr_args(tcp, &args, addr, size, false);
		}
	} else {
		struct xattr_args *args = get_tcb_priv_data(tcp);

		/* args */
		if (args)
			print_xattr_args(tcp, args, addr, size, true);
		else
			printaddr(addr);
	}

	/* size */
	tprint_arg_next();
	PRINT_VAL_U(size);

	return RVAL_DECODED;
}

SYS_FUNC(listxattrat)
{
	if (entering(tcp)) {
		/* dirfd, pathname, flags */
		decode_dirfd_pathname_flags(tcp);
	} else {
		print_xattr_list(tcp, tcp->u_arg[3], tcp->u_arg[4]);
	}
	return 0;
}

SYS_FUNC(removexattrat)
{
	/* dirfd, pathname, flags, name */
	decode_dirfd_pathname_flags_name(tcp);
	return RVAL_DECODED;
}
