/*
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_sysctl_args)
#include <linux/sysctl.h>
typedef struct __sysctl_args struct_sysctl_args;
#include MPERS_DEFS

SYS_FUNC(sysctl)
{
	struct_sysctl_args info;

	if (umove_or_printaddr(tcp, tcp->u_arg[0], &info))
		return RVAL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_PTR(info, name);
	tprint_struct_next();
	PRINT_FIELD_D(info, nlen);
	tprint_struct_next();
	PRINT_FIELD_PTR(info, oldval);
	tprint_struct_next();
	PRINT_FIELD_PTR(info, oldlenp);
	tprint_struct_next();
	PRINT_FIELD_PTR(info, newval);
	tprint_struct_next();
	PRINT_FIELD_U(info, newlen);
	tprint_struct_end();

	return RVAL_DECODED;
}
