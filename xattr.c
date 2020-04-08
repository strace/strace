/*
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2005-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_SYS_XATTR_H
# include <sys/xattr.h>
#endif

#include "xlat/xattrflags.h"

#ifndef XATTR_SIZE_MAX
# define XATTR_SIZE_MAX 65536
#endif

static void
print_xattr_val(struct tcb *const tcp,
		const kernel_ulong_t addr,
		const kernel_ulong_t insize,
		const kernel_ulong_t size)
{
	tprints(", ");

	if (size > XATTR_SIZE_MAX)
		printaddr(addr);
	else
		printstr_ex(tcp, addr, size, QUOTE_OMIT_TRAILING_0);
	tprintf(", %" PRI_klu, insize);
}

SYS_FUNC(setxattr)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1]);
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
	tprints(", ");
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	return RVAL_DECODED;
}

SYS_FUNC(fsetxattr)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1]);
	print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[3]);
	tprints(", ");
	printflags(xattrflags, tcp->u_arg[4], "XATTR_???");
	return RVAL_DECODED;
}

SYS_FUNC(getxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1]);
	} else {
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_rval);
	}
	return 0;
}

SYS_FUNC(fgetxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printstr(tcp, tcp->u_arg[1]);
	} else {
		print_xattr_val(tcp, tcp->u_arg[2], tcp->u_arg[3], tcp->u_rval);
	}
	return 0;
}

static void
print_xattr_list(struct tcb *const tcp, const kernel_ulong_t addr,
		 const kernel_ulong_t size)
{
	if (!size || syserror(tcp)) {
		printaddr(addr);
	} else {
		printstrn(tcp, addr, tcp->u_rval);
	}
	tprintf(", %" PRI_klu, size);
}

SYS_FUNC(listxattr)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(flistxattr)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_xattr_list(tcp, tcp->u_arg[1], tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(removexattr)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}

SYS_FUNC(fremovexattr)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printstr(tcp, tcp->u_arg[1]);
	return RVAL_DECODED;
}
