/*
 * Copyright (c) 2013 Christian Svensson <blue@cmd.nu>
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef OR1K

# define OR1K_ATOMIC_SWAP	1
# define OR1K_ATOMIC_CMPXCHG	2
# define OR1K_ATOMIC_XCHG	3
# define OR1K_ATOMIC_ADD		4
# define OR1K_ATOMIC_DECPOS	5
# define OR1K_ATOMIC_AND		6
# define OR1K_ATOMIC_OR		7
# define OR1K_ATOMIC_UMAX	8
# define OR1K_ATOMIC_UMIN	9

# include "xlat/atomic_ops.h"

SYS_FUNC(or1k_atomic)
{
	/* type */
	printxval64(atomic_ops, tcp->u_arg[0], "???");

	switch (tcp->u_arg[0]) {
	case OR1K_ATOMIC_DECPOS:
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[1]);
		break;

	case OR1K_ATOMIC_SWAP:
	case OR1K_ATOMIC_XCHG:
	case OR1K_ATOMIC_ADD:
	case OR1K_ATOMIC_AND:
	case OR1K_ATOMIC_OR:
	case OR1K_ATOMIC_UMAX:
	case OR1K_ATOMIC_UMIN:
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[1]);
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[2]);
		break;

	case OR1K_ATOMIC_CMPXCHG:
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[1]);
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[2]);
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[3]);
		break;

	default:
		break;
	}

	return RVAL_DECODED | RVAL_HEX;
}

#endif /* OR1K */
