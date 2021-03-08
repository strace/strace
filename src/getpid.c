/*
 * Copyright (c) 2020-2021 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

SYS_FUNC(getpid)
{
	return RVAL_DECODED | RVAL_TGID;
}

SYS_FUNC(gettid)
{
	return RVAL_DECODED | RVAL_TID;
}

SYS_FUNC(getpgrp)
{
	return RVAL_DECODED | RVAL_PGID;
}

SYS_FUNC(getpgid)
{
	printpid(tcp, tcp->u_arg[0], PT_TGID);

	return RVAL_DECODED | RVAL_PGID;
}

SYS_FUNC(getsid)
{
	printpid(tcp, tcp->u_arg[0], PT_TGID);

	return RVAL_DECODED | RVAL_SID;
}

SYS_FUNC(setpgid)
{
	/* pid */
	printpid(tcp, tcp->u_arg[0], PT_TGID);
	tprint_arg_next();

	/* pgid */
	printpid(tcp, tcp->u_arg[1], PT_PGID);

	return RVAL_DECODED;
}
