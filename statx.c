/*
 * Copyright (c) 2017-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "statx.h"

#include <sys/stat.h>

#include "xlat/statx_masks.h"
#include "xlat/statx_attrs.h"
#include "xlat/at_statx_sync_types.h"

static void
print_statx_timestamp(const struct_statx_timestamp *const p)
{
	PRINT_FIELD_D("{", *p, tv_sec);
	PRINT_FIELD_U(", ", *p, tv_nsec);
	tprints("}");
	tprints_comment(sprinttime_nsec(p->tv_sec,
		zero_extend_signed_to_ull(p->tv_nsec)));
}

SYS_FUNC(statx)
{
	if (entering(tcp)) {
		print_dirfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		printpath(tcp, tcp->u_arg[1]);
		tprints(", ");

		unsigned int flags = tcp->u_arg[2];
		printflags(at_statx_sync_types, flags & AT_STATX_SYNC_TYPE,
			   NULL);
		flags &= ~AT_STATX_SYNC_TYPE;
		if (flags) {
			tprints("|");
			printflags(at_flags, flags, NULL);
		}

		tprints(", ");
		printflags(statx_masks, tcp->u_arg[3], "STATX_???");
		tprints(", ");
	} else {
		struct_statx stx;
		if (umove_or_printaddr(tcp, tcp->u_arg[4], &stx))
			return 0;

		PRINT_FIELD_FLAGS("{", stx, stx_mask, statx_masks,
				  "STATX_???");

		if (!abbrev(tcp))
			PRINT_FIELD_U(", ", stx, stx_blksize);

		PRINT_FIELD_FLAGS(", ", stx, stx_attributes, statx_attrs,
				  "STATX_ATTR_???");

		if (!abbrev(tcp)) {
			PRINT_FIELD_U(", ", stx, stx_nlink);
			PRINT_FIELD_UID(", ", stx, stx_uid);
			PRINT_FIELD_UID(", ", stx, stx_gid);
		}

		PRINT_FIELD_OBJ_VAL(", ", stx, stx_mode, print_symbolic_mode_t);

		if (!abbrev(tcp))
			PRINT_FIELD_U(", ", stx, stx_ino);

		PRINT_FIELD_U(", ", stx, stx_size);

		if (!abbrev(tcp)) {
			PRINT_FIELD_U(", ", stx, stx_blocks);
			PRINT_FIELD_FLAGS(", ", stx, stx_attributes_mask,
					  statx_attrs, "STATX_ATTR_???");
			PRINT_FIELD_OBJ_PTR(", ", stx, stx_atime,
					    print_statx_timestamp);
			PRINT_FIELD_OBJ_PTR(", ", stx, stx_btime,
					    print_statx_timestamp);
			PRINT_FIELD_OBJ_PTR(", ", stx, stx_ctime,
					    print_statx_timestamp);
			PRINT_FIELD_OBJ_PTR(", ", stx, stx_mtime,
					    print_statx_timestamp);
			PRINT_FIELD_U(", ", stx, stx_rdev_major);
			PRINT_FIELD_U(", ", stx, stx_rdev_minor);
			PRINT_FIELD_U(", ", stx, stx_dev_major);
			PRINT_FIELD_U(", ", stx, stx_dev_minor);
		} else {
			tprints(", ...");
		}
		tprints("}");
	}
	return 0;
}
