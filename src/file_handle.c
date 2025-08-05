/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fcntl.h>
#include "xlat/name_to_handle_at_flags.h"

#ifndef MAX_HANDLE_SZ
# define MAX_HANDLE_SZ 128
#endif

typedef struct {
	unsigned int handle_bytes;
	int handle_type;
} file_handle_header;

static void
print_f_handle(struct tcb *tcp, kernel_ulong_t addr, unsigned int handle_bytes)
{
	unsigned int len = MIN(handle_bytes, MAX_HANDLE_SZ);
	char f_handle[MAX_HANDLE_SZ];
	addr += sizeof(file_handle_header);
	if (addr > sizeof(file_handle_header) &&
	    !umoven(tcp, addr, len, f_handle)) {
		print_quoted_string(f_handle, len, QUOTE_FORCE_HEX);
		if (handle_bytes > len)
			tprint_more_data_follows();
	} else {
		tprint_unavailable();
	}
}

SYS_FUNC(name_to_handle_at)
{
	file_handle_header h;
	const kernel_ulong_t addr = tcp->u_arg[2];

	if (entering(tcp)) {
		/* dirfd */
		tprints_arg_name("dirfd");
		print_dirfd(tcp, tcp->u_arg[0]);

		/* pathname */
		tprints_arg_next_name("pathname");
		printpath(tcp, tcp->u_arg[1]);

		/* handle */
		tprints_arg_next_name("handle");
		if (umove_or_printaddr(tcp, addr, &h)) {
			/* mount_id */
			tprints_arg_next_name("mount_id");
			printaddr(tcp->u_arg[3]);

			/* flags */
			tprints_arg_next_name("flags");
			printflags(name_to_handle_at_flags, tcp->u_arg[4],
				   "AT_???");

			return RVAL_DECODED;
		}

		tprint_struct_begin();
		PRINT_FIELD_U(h, handle_bytes);

		set_tcb_priv_ulong(tcp, h.handle_bytes);

		return 0;
	} else {
		if ((!syserror(tcp) || EOVERFLOW == tcp->u_error)
		    && !umove(tcp, addr, &h)) {

			if (h.handle_bytes != get_tcb_priv_ulong(tcp)) {
				tprint_value_changed();
				PRINT_VAL_U(h.handle_bytes);
			}
			if (!syserror(tcp)) {
				tprint_struct_next();
				PRINT_FIELD_D(h, handle_type);
				tprint_struct_next();
				tprints_field_name("f_handle");
				print_f_handle(tcp, addr, h.handle_bytes);
			}
		}
		tprint_struct_end();

		/* mount_id */
		tprints_arg_next_name("mount_id");
		printnum_int(tcp, tcp->u_arg[3], "%d");

		/* flags */
		tprints_arg_next_name("flags");
		printflags(name_to_handle_at_flags, tcp->u_arg[4], "AT_???");
	}
	return 0;
}

SYS_FUNC(open_by_handle_at)
{
	file_handle_header h;
	const kernel_ulong_t addr = tcp->u_arg[1];

	/* mount_fd */
	tprints_arg_name("mount_fd");
	printfd(tcp, tcp->u_arg[0]);

	/* handle */
	tprints_arg_next_name("handle");
	if (!umove_or_printaddr(tcp, addr, &h)) {
		tprint_struct_begin();
		PRINT_FIELD_U(h, handle_bytes);
		tprint_struct_next();
		PRINT_FIELD_D(h, handle_type);
		tprint_struct_next();
		tprints_field_name("f_handle");
		print_f_handle(tcp, addr, h.handle_bytes);
		tprint_struct_end();
	}

	/* flags */
	tprints_arg_next_name("flags");
	tprint_open_modes(tcp->u_arg[2]);

	return RVAL_DECODED | RVAL_FD;
}
