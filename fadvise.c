/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Daniel Jacobowitz  <dan@codesourcery.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2009 Kirill A. Shutemov <kirill@shutemov.name>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <fcntl.h>

#include "xlat/advise.h"

SYS_FUNC(fadvise64)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld", 1);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[argn++]);
	printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");

	return RVAL_DECODED;
}

SYS_FUNC(fadvise64_64)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld, ", 1);
	argn = printllval(tcp, "%lld, ", argn);
#if defined __ARM_EABI__ || defined AARCH64 || defined POWERPC || defined XTENSA
	printxval(advise, tcp->u_arg[1], "POSIX_FADV_???");
#else
	printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");
#endif

	return RVAL_DECODED;
}
