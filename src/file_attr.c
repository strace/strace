/*
 * Copyright (c) 2025 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>
#include <linux/fs.h>
#include "xlat/file_attr_flags.h"

static void
decode_dirfd_pathname(struct tcb *tcp)
{
	/* dirfd */
	tprints_arg_name("dirfd");
	print_dirfd(tcp, tcp->u_arg[0]);

	/* pathname */
	tprints_arg_next_name("pathname");
	printpath(tcp, tcp->u_arg[1]);
}

static void
decode_flags(struct tcb *tcp)
{
	/* flags */
	tprints_arg_next_name("flags");
	printflags(file_attr_flags, tcp->u_arg[4], "AT_???");
}

static void
decode_file_attr(struct tcb *const tcp, const bool is_get)
{
	const kernel_ulong_t addr = tcp->u_arg[2];
	const kernel_ulong_t usize = tcp->u_arg[3];
	const unsigned long page_size = get_pagesize();
	struct file_attr fa = { 0 };
	const size_t fetch_size = MIN(sizeof(fa), usize);

	static_assert(sizeof(fa) >= FILE_ATTR_SIZE_VER0,
		      "sizeof(struct file_attr) < FILE_ATTR_SIZE_VER0");

	tprints_arg_next_name("attr");
	if (fetch_size < FILE_ATTR_SIZE_VER0 || usize > page_size) {
		printaddr(addr);
	} else if (!umoven_or_printaddr(tcp, addr, fetch_size, &fa)) {
		tprint_struct_begin();
		PRINT_FIELD_FLAGS(fa, fa_xflags, fs_xflags, "FS_XFLAG_???");

		tprint_struct_next();
		PRINT_FIELD_U(fa, fa_extsize);

		if (is_get) {
			tprint_struct_next();
			PRINT_FIELD_U(fa, fa_nextents);
		}

		tprint_struct_next();
		PRINT_FIELD_X(fa, fa_projid);

		tprint_struct_next();
		PRINT_FIELD_U(fa, fa_cowextsize);

		if (usize > FILE_ATTR_SIZE_VER0) {
			print_nonzero_bytes(tcp, tprint_struct_next, addr,
					    FILE_ATTR_SIZE_VER0, usize,
					    QUOTE_FORCE_HEX);
		}

		tprint_struct_end();
	}

	tprints_arg_next_name("size");
	PRINT_VAL_U(usize);
}

SYS_FUNC(file_getattr)
{
	if (entering(tcp)) {
		decode_dirfd_pathname(tcp);
	} else {
		decode_file_attr(tcp, true);
		decode_flags(tcp);
	}

	return 0;
}

SYS_FUNC(file_setattr)
{
	decode_dirfd_pathname(tcp);
	decode_file_attr(tcp, false);
	decode_flags(tcp);

	return RVAL_DECODED;
}
