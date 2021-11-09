/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/landlock.h>

#include "xlat/landlock_create_ruleset_flags.h"
#include "xlat/landlock_rule_types.h"
#include "xlat/landlock_ruleset_access_fs.h"

static void
print_landlock_ruleset_attr(struct tcb *tcp, const kernel_ulong_t addr,
			    const kernel_ulong_t size)
{
	struct landlock_ruleset_attr attr;

	if (size < offsetofend(typeof(attr), handled_access_fs)) {
		printaddr(addr);
		return;
	}

	if (umoven_or_printaddr(tcp, addr, MIN(size, sizeof(attr)), &attr))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(attr, handled_access_fs, landlock_ruleset_access_fs,
			  "LANDLOCK_ACCESS_FS_???");

	if (size > offsetofend(typeof(attr), handled_access_fs)) {
		tprint_arg_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();
}

SYS_FUNC(landlock_create_ruleset)
{
	kernel_ulong_t attr = tcp->u_arg[0];
	kernel_ulong_t size = tcp->u_arg[1];
	unsigned int flags = tcp->u_arg[2];
	int fd_flag = flags & LANDLOCK_CREATE_RULESET_VERSION ? 0 : RVAL_FD;

	/* attr */
	print_landlock_ruleset_attr(tcp, attr, size);
	tprint_arg_next();

	/* size */
	PRINT_VAL_U(size);
	tprint_arg_next();

	/* flags */
	printflags(landlock_create_ruleset_flags, flags,
		   "LANDLOCK_CREATE_RULESET_???");

	return RVAL_DECODED | fd_flag;
}

static void
print_landlock_path_beneath_attr(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct landlock_path_beneath_attr attr;

	if (umove_or_printaddr(tcp, addr, &attr))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(attr, allowed_access, landlock_ruleset_access_fs,
			  "LANDLOCK_ACCESS_FS_???");
	tprint_struct_next();
	PRINT_FIELD_FD(attr, parent_fd, tcp);
	tprint_struct_end();
}

SYS_FUNC(landlock_add_rule)
{
	unsigned int rule_type = tcp->u_arg[1];

	/* ruleset_fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* rule_type */
	printxval(landlock_rule_types, rule_type, "LANDLOCK_RULE_???");
	tprint_arg_next();

	/* rule_attr */
	switch (rule_type) {
	case LANDLOCK_RULE_PATH_BENEATH:
		print_landlock_path_beneath_attr(tcp, tcp->u_arg[2]);
		break;

	default:
		printaddr(tcp->u_arg[2]);
	}
	tprint_arg_next();

	/* flags */
	PRINT_VAL_X((unsigned int) tcp->u_arg[3]);

	return RVAL_DECODED;
}

SYS_FUNC(landlock_restrict_self)
{
	/* ruleset_fd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* flags */
	PRINT_VAL_X((unsigned int) tcp->u_arg[1]);

	return RVAL_DECODED;
}
