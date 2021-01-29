/*
 * Copyright (c) 2014-2018 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "statfs.h"
#include "xlat/fsmagic.h"
#include "xlat/statfs_flags.h"

#if defined HAVE_STRUCT_STATFS_F_FSID_VAL \
 || defined HAVE_STRUCT_STATFS_F_FSID___VAL \
 || defined HAVE_STRUCT_STATFS64_F_FSID_VAL \
 || defined HAVE_STRUCT_STATFS64_F_FSID___VAL
static void
print_f_fsid(const typeof_field(struct strace_statfs, f_fsid) *const p,
	     struct tcb *const tcp)
{
	tprint_struct_begin();
	PRINT_FIELD_ARRAY(*p, val, tcp, print_xint64_array_member);
	tprints("}");
}
#endif

void
print_struct_statfs(struct tcb *const tcp, const kernel_ulong_t addr)
{
#ifdef HAVE_STRUCT_STATFS
	struct strace_statfs b;

	if (!fetch_struct_statfs(tcp, addr, &b))
		return;

	PRINT_FIELD_XVAL("{", b, f_type, fsmagic, NULL);
	PRINT_FIELD_U(", ", b, f_bsize);
	PRINT_FIELD_U(", ", b, f_blocks);
	PRINT_FIELD_U(", ", b, f_bfree);
	PRINT_FIELD_U(", ", b, f_bavail);
	PRINT_FIELD_U(", ", b, f_files);
	PRINT_FIELD_U(", ", b, f_ffree);
# if defined HAVE_STRUCT_STATFS_F_FSID_VAL \
  || defined HAVE_STRUCT_STATFS_F_FSID___VAL
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(b, f_fsid, print_f_fsid, tcp);
# endif
	PRINT_FIELD_U(", ", b, f_namelen);
# ifdef HAVE_STRUCT_STATFS_F_FRSIZE
	PRINT_FIELD_U(", ", b, f_frsize);
# endif
# ifdef HAVE_STRUCT_STATFS_F_FLAGS
	if (b.f_flags & ST_VALID) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(b, f_flags, statfs_flags, "ST_???");
	}
# endif
	tprints("}");
#else
	printaddr(addr);
#endif
}

void
print_struct_statfs64(struct tcb *const tcp, const kernel_ulong_t addr,
		      const kernel_ulong_t size)
{
#ifdef HAVE_STRUCT_STATFS64
	struct strace_statfs b;

	if (!fetch_struct_statfs64(tcp, addr, size, &b))
		return;

	PRINT_FIELD_XVAL("{", b, f_type, fsmagic, NULL);
	PRINT_FIELD_U(", ", b, f_bsize);
	PRINT_FIELD_U(", ", b, f_blocks);
	PRINT_FIELD_U(", ", b, f_bfree);
	PRINT_FIELD_U(", ", b, f_bavail);
	PRINT_FIELD_U(", ", b, f_files);
	PRINT_FIELD_U(", ", b, f_ffree);
# if defined HAVE_STRUCT_STATFS64_F_FSID_VAL \
  || defined HAVE_STRUCT_STATFS64_F_FSID___VAL
	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(b, f_fsid, print_f_fsid, tcp);
# endif
	PRINT_FIELD_U(", ", b, f_namelen);
# ifdef HAVE_STRUCT_STATFS64_F_FRSIZE
	PRINT_FIELD_U(", ", b, f_frsize);
# endif
# ifdef HAVE_STRUCT_STATFS64_F_FLAGS
	if (b.f_flags & ST_VALID) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(b, f_flags, statfs_flags, "ST_???");
	}
# endif
	tprints("}");
#else
	printaddr(addr);
#endif
}
