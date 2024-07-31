/*
 * Copyright (c) 2017-2024 The strace developers.
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
		tprint_flags_begin();
		printflags_in(at_statx_sync_types, flags & AT_STATX_SYNC_TYPE,
			      NULL);
		flags &= ~AT_STATX_SYNC_TYPE;
		if (flags) {
			tprint_flags_or();
			printflags_in(at_flags, flags, NULL);
		}
		tprint_flags_end();
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
			if (stx.stx_mask & STATX_NLINK) {
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_nlink);
			}
			if (stx.stx_mask & STATX_UID) {
				tprint_struct_next();
				PRINT_FIELD_ID(stx, stx_uid);
			}
			if (stx.stx_mask & STATX_GID) {
				tprint_struct_next();
				PRINT_FIELD_ID(stx, stx_gid);
			}
		}

		if (stx.stx_mask & (STATX_TYPE|STATX_MODE)) {
			tprint_struct_next();
			PRINT_FIELD_OBJ_VAL(stx, stx_mode,
					    print_symbolic_mode_t);
		}

		if (!abbrev(tcp)) {
			if (stx.stx_mask & STATX_INO) {
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_ino);
			}
		}

		if (stx.stx_mask & STATX_SIZE) {
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_size);
		}

		if (!abbrev(tcp)) {
			if (stx.stx_mask & STATX_BLOCKS) {
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_blocks);
			}
			tprint_struct_next();
			PRINT_FIELD_FLAGS(stx, stx_attributes_mask,
					  statx_attrs, "STATX_ATTR_???");
			if (stx.stx_mask & STATX_ATIME) {
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(stx, stx_atime,
						    print_statx_timestamp);
			}
			if (stx.stx_mask & STATX_BTIME) {
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(stx, stx_btime,
						    print_statx_timestamp);
			}
			if (stx.stx_mask & STATX_CTIME) {
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(stx, stx_ctime,
						    print_statx_timestamp);
			}
			if (stx.stx_mask & STATX_MTIME) {
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(stx, stx_mtime,
						    print_statx_timestamp);
			}
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_rdev_major);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_rdev_minor);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_dev_major);
			tprint_struct_next();
			PRINT_FIELD_U(stx, stx_dev_minor);
			if (stx.stx_mask & STATX_MNT_ID) {
				tprint_struct_next();
				PRINT_FIELD_X(stx, stx_mnt_id);
			}
			if (stx.stx_mask & STATX_DIOALIGN) {
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_dio_mem_align);
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_dio_offset_align);
			}
			if (stx.stx_mask & STATX_SUBVOL) {
				tprint_struct_next();
				PRINT_FIELD_X(stx, stx_subvol);
			}
			if (stx.stx_attributes & STATX_ATTR_WRITE_ATOMIC) {
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_atomic_write_unit_min);
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_atomic_write_unit_max);
				tprint_struct_next();
				PRINT_FIELD_U(stx, stx_atomic_write_segments_max);
			}
		} else {
			tprint_struct_next();
			tprint_more_data_follows();
		}
		tprint_struct_end();
	}
	return 0;
}
