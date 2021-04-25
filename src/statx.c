/*
 * Copyright (c) 2017-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "statx.h"

#include <sys/stat.h>
#include <linux/fcntl.h>

#include "xlat/statx_masks.h"
#include "xlat/statx_attrs.h"
#include "xlat/at_statx_sync_types.h"

static void
print_statx_timestamp(const struct_statx_timestamp *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_D(*p, tv_sec);
	tprint_struct_next();
	PRINT_FIELD_U(*p, tv_nsec);
	tprint_struct_end();
	tprints_comment(sprinttime_nsec(p->tv_sec,
		zero_extend_signed_to_ull(p->tv_nsec)));
}

SYS_FUNC(statx)
{
	if (entering(tcp)) {
		/* dirfd */
		print_dirfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		/* pathname */
		printpath(tcp, tcp->u_arg[1]);
		tprint_arg_next();

		/* flags */
		unsigned int flags = tcp->u_arg[2];
		printflags(at_statx_sync_types, flags & AT_STATX_SYNC_TYPE,
			   NULL);
		flags &= ~AT_STATX_SYNC_TYPE;
		if (flags) {
			tprints("|");
			printflags(at_flags, flags, NULL);
		}
		tprint_arg_next();

		/* mask */
		printflags(statx_masks, tcp->u_arg[3], "STATX_???");
		tprint_arg_next();
	} else {
		/* statxbuf */
		struct_statx stx;
		if (umove_or_printaddr(tcp, tcp->u_arg[4], &stx))
			return 0;

		tprint_struct_begin();
		PRINT_FIELD_FLAGS(stx, stx_mask, statx_masks,
				  "STATX_???");

		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_blksize);
		}

		tprint_struct_next();
		PRINT_FIELD_FLAGS(stx, stx_attributes, statx_attrs,
				  "STATX_ATTR_???");

		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_nlink);
			tprint_struct_next();
			PRINT_FIELD_ID(stx, stx_uid);
			tprint_struct_next();
			PRINT_FIELD_ID(stx, stx_gid);
		}

		tprint_struct_next();
		PRINT_FIELD_OBJ_VAL(stx, stx_mode, print_symbolic_mode_t);

		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_ino);
		}

		tprint_struct_next();
		PRINT_FIELD_U(stx, stx_size);

		if (!abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_blocks);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(stx, stx_attributes_mask,
					  statx_attrs, "STATX_ATTR_???");
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(stx, stx_atime,
					    print_statx_timestamp);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(stx, stx_btime,
					    print_statx_timestamp);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(stx, stx_ctime,
					    print_statx_timestamp);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(stx, stx_mtime,
					    print_statx_timestamp);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_rdev_major);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_rdev_minor);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_dev_major);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_dev_minor);
		} else {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
	return 0;
}
