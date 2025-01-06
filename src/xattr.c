/*
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/xattr.h>

#include "xlat/xattrflags.h"

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
	printstr(tcp, tcp->u_arg[1]);
	tprint_arg_next();

	/* value */
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3]);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(tcp->u_arg[3]);
	tprint_arg_next();

	/* flags */
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");

	return RVAL_DECODED;
}

SYS_FUNC(setxattr)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	return decode_setxattr_without_path(tcp);
}

SYS_FUNC(fsetxattr)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	return decode_setxattr_without_path(tcp);
}

static int
decode_getxattr_without_path(struct tcb *const tcp)
{
	if (entering(tcp)) {
		/* name */
		printstr(tcp, tcp->u_arg[1]);
		tprint_arg_next();

		if (tcp->u_arg[3])
			return 0;

		/* value */
		printaddr(tcp->u_arg[2]);
		tprint_arg_next();

		/* size */
		PRINT_VAL_U(tcp->u_arg[3]);
	} else {
		/* value */
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_rval);
		tprint_arg_next();

		/* size */
		PRINT_VAL_U(tcp->u_arg[3]);
	}

	return RVAL_DECODED;
}

SYS_FUNC(getxattr)
{
	if (entering(tcp)) {
		/* pathname */
		printpath(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	}

	return decode_getxattr_without_path(tcp);
}

SYS_FUNC(fgetxattr)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	}

	return decode_getxattr_without_path(tcp);
}

static void
print_xattr_list(struct tcb *const tcp, const kernel_ulong_t addr,
		 const kernel_ulong_t size)
{
	/* list */
	if (!size || syserror(tcp)) {
		printaddr(addr);
	} else {
		printstrn(tcp, addr, tcp->u_rval);
	}
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(size);
}

SYS_FUNC(listxattr)
{
	if (entering(tcp)) {
		/* pathname */
		printpath(tcp, tcp->u_arg[0]);
		tprint_arg_next();
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
		tprint_arg_next();
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(removexattr)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* name */
	printstr(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}

SYS_FUNC(fremovexattr)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* name */
	printstr(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}
