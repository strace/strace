/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/name_to_handle_at_flags.h"

#ifndef MAX_HANDLE_SZ
# define MAX_HANDLE_SZ 128
#endif

typedef struct {
	unsigned int handle_bytes;
	int handle_type;
} file_handle_header;

SYS_FUNC(name_to_handle_at)
{
	file_handle_header h;
	const kernel_ulong_t addr = tcp->u_arg[2];

	if (entering(tcp)) {
		/* dirfd */
		print_dirfd(tcp, tcp->u_arg[0]);
		tprints(", ");

		/* pathname */
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");

		/* handle */
		if (umove_or_printaddr(tcp, addr, &h)) {
			tprints(", ");

			/* mount_id */
			printaddr(tcp->u_arg[3]);
			tprints(", ");

			/* flags */
			printflags(name_to_handle_at_flags, tcp->u_arg[4],
				   "AT_???");

			return RVAL_DECODED;
		}
		tprintf("{handle_bytes=%u", h.handle_bytes);

		set_tcb_priv_ulong(tcp, h.handle_bytes);

		return 0;
	} else {
		unsigned int i = get_tcb_priv_ulong(tcp);

		if ((!syserror(tcp) || EOVERFLOW == tcp->u_error)
		    && !umove(tcp, addr, &h)) {
			unsigned char f_handle[MAX_HANDLE_SZ];

			if (i != h.handle_bytes)
				tprintf(" => %u", h.handle_bytes);
			if (!syserror(tcp)) {
				tprintf(", handle_type=%d", h.handle_type);
				if (h.handle_bytes > MAX_HANDLE_SZ)
					h.handle_bytes = MAX_HANDLE_SZ;
				if (!umoven(tcp, addr + sizeof(h), h.handle_bytes,
					    f_handle)) {
					tprints(", f_handle=0x");
					for (i = 0; i < h.handle_bytes; ++i)
						tprintf("%02x", f_handle[i]);
				}
			}
		}
		tprints("}, ");

		/* mount_id */
		printnum_int(tcp, tcp->u_arg[3], "%d");
		tprints(", ");

		/* flags */
		printflags(name_to_handle_at_flags, tcp->u_arg[4], "AT_???");
	}
	return 0;
}

SYS_FUNC(open_by_handle_at)
{
	file_handle_header h;
	const kernel_ulong_t addr = tcp->u_arg[1];

	/* mount_fd */
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");

	/* handle */
	if (!umove_or_printaddr(tcp, addr, &h)) {
		unsigned char f_handle[MAX_HANDLE_SZ];

		tprintf("{handle_bytes=%u, handle_type=%d",
			h.handle_bytes, h.handle_type);
		if (h.handle_bytes > MAX_HANDLE_SZ)
			h.handle_bytes = MAX_HANDLE_SZ;
		if (!umoven(tcp, addr + sizeof(h), h.handle_bytes, &f_handle)) {
			unsigned int i;

			tprints(", f_handle=0x");
			for (i = 0; i < h.handle_bytes; ++i)
				tprintf("%02x", f_handle[i]);
		}
		tprints("}");
	}
	tprints(", ");

	/* flags */
	tprint_open_modes(tcp->u_arg[2]);

	return RVAL_DECODED;
}
