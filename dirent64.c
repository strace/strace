/*
 * Copyright (c) 2020 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "xgetdents.h"
#include "kernel_dirent.h"
#include "print_fields.h"

#define D_NAME_LEN_MAX 256

static void
print_dentry_head(const kernel_dirent64_t *const dent)
{
	PRINT_FIELD_U("{", *dent, d_ino);
	PRINT_FIELD_U(", ", *dent, d_off);
	PRINT_FIELD_U(", ", *dent, d_reclen);
}

static unsigned int
decode_dentry_head(struct tcb *const tcp, const void *const arg)
{
	const kernel_dirent64_t *const dent = arg;

	if (!abbrev(tcp))
		print_dentry_head(dent);

	return dent->d_reclen;
}

static int
decode_dentry_tail(struct tcb *const tcp, kernel_ulong_t addr,
		   const void *const arg, unsigned int d_name_len)
{
	const kernel_dirent64_t *const dent = arg;
	int rc = 0;

	/* !abbrev(tcp) */

	PRINT_FIELD_XVAL(", ", *dent, d_type, dirent_types, "DT_???");

	if (d_name_len) {
		if (d_name_len > D_NAME_LEN_MAX)
			d_name_len = D_NAME_LEN_MAX;
		tprints(", d_name=");
		rc = printpathn(tcp, addr, d_name_len - 1);
	}
	tprints("}");

	return rc;
}

SYS_FUNC(getdents64)
{
	/* The minimum size of a valid directory entry.  */
	static const unsigned int header_size =
		offsetof(kernel_dirent64_t, d_name);

	return xgetdents(tcp, header_size,
			 decode_dentry_head, decode_dentry_tail);
}
