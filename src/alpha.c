/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef ALPHA

# include "xstring.h"

static int
decode_getxxid(struct tcb *tcp, const char *what)
{
	if (entering(tcp))
		return 0;

	long rval = getrval2(tcp);
	if (rval == -1)
		return 0;
	static const char const fmt[] = "%s %ld";
	static char outstr[sizeof(fmt) + 3 * sizeof(rval)];
	xsprintf(outstr, fmt, what, rval);
	tcp->auxstr = outstr;
	return RVAL_STR;
}

SYS_FUNC(getxpid)
{
	return decode_getxxid(tcp, "ppid");
}

SYS_FUNC(getxuid)
{
	return decode_getxxid(tcp, "euid");
}

SYS_FUNC(getxgid)
{
	return decode_getxxid(tcp, "egid");
}

SYS_FUNC(osf_statfs)
{
	/* pathname */
	printpath(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* buf */
	printaddr(tcp->u_arg[1]);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(tcp->u_arg[2]);

	return RVAL_DECODED;
}

SYS_FUNC(osf_fstatfs)
{
	/* fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* buf */
	printaddr(tcp->u_arg[1]);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(tcp->u_arg[2]);

	return RVAL_DECODED;
}

#endif /* ALPHA */
