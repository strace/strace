/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "kernel_dirent.h"

#include DEF_MPERS_TYPE(kernel_dirent_t)

#include MPERS_DEFS

#include "xgetdents.h"

#define D_NAME_LEN_MAX 256

/* The minimum size of a valid directory entry.  */
static const unsigned int header_size =
	offsetof(kernel_dirent_t, d_name);

static void
print_dentry_head(const kernel_dirent_t *const dent)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*dent, d_ino);
	tprint_struct_next();
	PRINT_FIELD_U(*dent, d_off);
	tprint_struct_next();
	PRINT_FIELD_U(*dent, d_reclen);
}

static unsigned int
decode_dentry_head(struct tcb *const tcp, const void *const arg)
{
	const kernel_dirent_t *const dent = arg;

	if (!abbrev(tcp))
		print_dentry_head(dent);

	return dent->d_reclen;
}

static int
decode_dentry_tail(struct tcb *const tcp, kernel_ulong_t addr,
		   const void *const arg, const unsigned int d_name_type_len)
{
	int rc = 0;

	/* !abbrev(tcp) */

	if (d_name_type_len) {
		unsigned int d_name_len = d_name_type_len - 1;
		if (d_name_len) {
			if (d_name_len > D_NAME_LEN_MAX)
				d_name_len = D_NAME_LEN_MAX;
			tprint_struct_next();
			tprints_field_name("d_name");
			rc = printpathn(tcp, addr, d_name_len);
		}
		tprint_struct_next();
		tprints_field_name("d_type");
		const kernel_ulong_t d_type_addr =
			addr + (d_name_type_len - 1);
		unsigned char d_type;
		if (umove_or_printaddr(tcp, d_type_addr, &d_type))
			rc = -1;
		else
			printxval(dirent_types, d_type, "DT_???");
	}
	tprint_struct_end();

	return rc;
}

SYS_FUNC(getdents)
{
	return xgetdents(tcp, header_size,
			 decode_dentry_head, decode_dentry_tail);
}

static void
print_old_dirent(struct tcb *const tcp, const kernel_ulong_t addr)
{
	kernel_dirent_t dent;

	if (umove_or_printaddr(tcp, addr, &dent))
		return;

	print_dentry_head(&dent);
	tprint_struct_next();
	tprints_field_name("d_name");
	printpathn(tcp, addr + header_size,
		   MIN(dent.d_reclen, D_NAME_LEN_MAX) + 1);
	tprint_struct_end();
}

SYS_FUNC(readdir)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
#ifdef ENABLE_SECONTEXT
		tcp->last_dirfd = (int) tcp->u_arg[0];
#endif
		tprint_arg_next();
	} else {
		/* dirp */
		if (tcp->u_rval == 0)
			printaddr(tcp->u_arg[1]);
		else
			print_old_dirent(tcp, tcp->u_arg[1]);

		/* count */
		const unsigned int count = tcp->u_arg[2];
		/* Not much point in printing this out, it is always 1. */
		if (count != 1) {
			tprint_arg_next();
			PRINT_VAL_U(count);
		}
	}
	return 0;
}
