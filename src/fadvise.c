/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Daniel Jacobowitz  <dan@codesourcery.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2009 Kirill A. Shutemov <kirill@shutemov.name>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>

#include "xlat/advise.h"

SYS_FUNC(fadvise64)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* offset */
	unsigned int argn = print_arg_lld(tcp, 1);
	tprint_arg_next();

	/* len */
	kernel_ulong_t len = tcp->u_arg[argn++];
	PRINT_VAL_U(len);
	tprint_arg_next();

	/* advice */
	printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");

	return RVAL_DECODED;
}

SYS_FUNC(fadvise64_64)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* offset */
	unsigned int argn = print_arg_lld(tcp, 1);
	tprint_arg_next();

	/* len */
	argn = print_arg_lld(tcp, argn);
	tprint_arg_next();

	/* advice */
#if defined __ARM_EABI__ || defined AARCH64 || defined POWERPC || defined XTENSA
	printxval(advise, tcp->u_arg[1], "POSIX_FADV_???");
#else
	printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");
#endif

	return RVAL_DECODED;
}
