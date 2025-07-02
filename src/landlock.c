/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2021-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/landlock.h>

#include "xlat/landlock_create_ruleset_flags.h"
#include "xlat/landlock_rule_types.h"
#include "xlat/landlock_ruleset_access_fs.h"
#include "xlat/landlock_ruleset_access_net.h"
#include "xlat/landlock_scope_flags.h"

static void
print_landlock_ruleset_attr(struct tcb *tcp, const kernel_ulong_t addr,
			    const kernel_ulong_t size)
{
	struct landlock_ruleset_attr attr = { 0 };
	const size_t min_attr_size =
		offsetofend(typeof(attr), handled_access_fs);
	const size_t max_attr_size =
		offsetofend(typeof(attr), scoped);

	if (size < min_attr_size) {
		printaddr(addr);
		return;
	}

	if (umoven_or_printaddr(tcp, addr, MIN(size, max_attr_size), &attr))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(attr, handled_access_fs, landlock_ruleset_access_fs,
			  "LANDLOCK_ACCESS_FS_???");

	if (size > offsetof(typeof(attr), handled_access_net)) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(attr, handled_access_net,
				  landlock_ruleset_access_net,
				  "LANDLOCK_ACCESS_NET_???");
	}

	if (size > offsetof(typeof(attr), scoped)) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(attr, scoped,
				  landlock_scope_flags,
				  "LANDLOCK_SCOPE_???");
	}

	if (size > max_attr_size) {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();
}

SYS_FUNC(landlock_create_ruleset)
{
	kernel_ulong_t attr = tcp->u_arg[0];
	kernel_ulong_t size = tcp->u_arg[1];
	unsigned int flags = tcp->u_arg[2];
	const unsigned int landlock_nofd_flags =
		LANDLOCK_CREATE_RULESET_VERSION | LANDLOCK_CREATE_RULESET_ERRATA;
	unsigned int fd_flag = flags & landlock_nofd_flags ? 0 : RVAL_FD;

	/* attr */
	tprints_arg_name("attr");
	print_landlock_ruleset_attr(tcp, attr, size);

	/* size */
	tprints_arg_next_name("size");
	PRINT_VAL_U(size);

	/* flags */
	tprints_arg_next_name("flags");
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

static void
print_landlock_net_port_attr(struct tcb *tcp, const kernel_ulong_t addr)
{
	struct landlock_net_port_attr attr;

	if (umove_or_printaddr(tcp, addr, &attr))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(attr, allowed_access, landlock_ruleset_access_net,
			  "LANDLOCK_ACCESS_NET_???");
	tprint_struct_next();
	PRINT_FIELD_U(attr, port);
	tprint_struct_end();
}

SYS_FUNC(landlock_add_rule)
{
	unsigned int rule_type = tcp->u_arg[1];

	/* ruleset_fd */
	tprints_arg_name("ruleset_fd");
	printfd(tcp, tcp->u_arg[0]);

	/* rule_type */
	tprints_arg_next_name("rule_type");
	printxval(landlock_rule_types, rule_type, "LANDLOCK_RULE_???");

	/* rule_attr */
	tprints_arg_next_name("rule_attr");
	switch (rule_type) {
	case LANDLOCK_RULE_PATH_BENEATH:
		print_landlock_path_beneath_attr(tcp, tcp->u_arg[2]);
		break;

	case LANDLOCK_RULE_NET_PORT:
		print_landlock_net_port_attr(tcp, tcp->u_arg[2]);
		break;

	default:
		printaddr(tcp->u_arg[2]);
	}

	/* flags */
	tprints_arg_next_name("flags");
	PRINT_VAL_X((unsigned int) tcp->u_arg[3]);

	return RVAL_DECODED;
}

SYS_FUNC(landlock_restrict_self)
{
	/* ruleset_fd */
	tprints_arg_name("ruleset_fd");
	printfd(tcp, tcp->u_arg[0]);

	/* flags */
	tprints_arg_next_name("flags");
	PRINT_VAL_X((unsigned int) tcp->u_arg[1]);

	return RVAL_DECODED;
}
