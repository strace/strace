/*
 * Copyright (c) 2020-2021 Ákos Uzonyi <uzonyi.akos@gmail.com>
 * Copyright (c) 2021-2025 The strace developers.
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
	tprints_arg_name("pid");
	printpid(tcp, tcp->u_arg[0], PT_TGID);

	return RVAL_DECODED | RVAL_PGID;
}

SYS_FUNC(getsid)
{
	tprints_arg_name("pid");
	printpid(tcp, tcp->u_arg[0], PT_TGID);

	return RVAL_DECODED | RVAL_SID;
}

SYS_FUNC(setpgid)
{
	/* pid */
	tprints_arg_name("pid");
	printpid(tcp, tcp->u_arg[0], PT_TGID);

	/* pgid */
	tprints_arg_next_name("pgid");
	printpid(tcp, tcp->u_arg[1], PT_PGID);

	return RVAL_DECODED;
}
