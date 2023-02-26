/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_btrfs_ioctl_dev_replace_args)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_send_args)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_received_subvol_args)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_timespec)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_vol_args_v2)

#include <linux/btrfs_tree.h>

typedef struct btrfs_ioctl_dev_replace_args
	struct_btrfs_ioctl_dev_replace_args;
typedef struct btrfs_ioctl_send_args
	struct_btrfs_ioctl_send_args;
typedef struct btrfs_ioctl_received_subvol_args
	struct_btrfs_ioctl_received_subvol_args;
typedef struct btrfs_ioctl_timespec
	struct_btrfs_ioctl_timespec;
typedef struct btrfs_ioctl_vol_args_v2
	struct_btrfs_ioctl_vol_args_v2;

#include MPERS_DEFS

#include <linux/fs.h>

#include "xlat/btrfs_balance_args.h"
#include "xlat/btrfs_balance_ctl_cmds.h"
#include "xlat/btrfs_balance_flags.h"
#include "xlat/btrfs_balance_state.h"
#include "xlat/btrfs_compress_types.h"
#include "xlat/btrfs_cont_reading_from_srcdev_mode.h"
#include "xlat/btrfs_csum_types.h"
#include "xlat/btrfs_defrag_flags.h"
#include "xlat/btrfs_dev_replace_cmds.h"
#include "xlat/btrfs_dev_replace_results.h"
#include "xlat/btrfs_dev_replace_state.h"
#include "xlat/btrfs_dev_stats_flags.h"
#include "xlat/btrfs_dev_stats_values.h"
#include "xlat/btrfs_features_compat.h"
#include "xlat/btrfs_features_compat_ro.h"
#include "xlat/btrfs_features_incompat.h"
#include "xlat/btrfs_fs_info_flags.h"
#include "xlat/btrfs_key_types.h"
#include "xlat/btrfs_logical_ino_args_flags.h"
#include "xlat/btrfs_qgroup_ctl_cmds.h"
#include "xlat/btrfs_qgroup_inherit_flags.h"
#include "xlat/btrfs_qgroup_limit_flags.h"
#include "xlat/btrfs_qgroup_status_flags.h"
#include "xlat/btrfs_scrub_flags.h"
#include "xlat/btrfs_send_flags.h"
#include "xlat/btrfs_snap_flags_v2.h"
#include "xlat/btrfs_space_info_flags.h"
#include "xlat/btrfs_tree_objectids.h"

static void
btrfs_print_balance_args(const struct btrfs_balance_args *const bba)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*bba, profiles, btrfs_space_info_flags,
			  "BTRFS_BLOCK_GROUP_???");
	tprint_struct_next();
	PRINT_FIELD_U64(*bba, usage);
	tprint_struct_next();
	PRINT_FIELD_DEV(*bba, devid);
	tprint_struct_next();
	PRINT_FIELD_U64(*bba, pstart);
	tprint_struct_next();
	PRINT_FIELD_U64(*bba, pend);
	tprint_struct_next();
	PRINT_FIELD_U64(*bba, vstart);
	tprint_struct_next();
	PRINT_FIELD_U64(*bba, vend);
	tprint_struct_next();
	PRINT_FIELD_U64(*bba, target);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*bba, flags, btrfs_balance_args,
			  "BTRFS_BALANCE_ARGS_???");
	tprint_struct_end();
}

static void
btrfs_print_balance(struct tcb *const tcp, const kernel_ulong_t arg, bool out)
{
	struct btrfs_ioctl_balance_args balance_args;

	if (umove_or_printaddr(tcp, arg, &balance_args))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(balance_args, flags, btrfs_balance_flags,
			  "BTRFS_BALANCE_???");
	if (out) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(balance_args, state,
				  btrfs_balance_state,
				  "BTRFS_BALANCE_STATE_???");
	}

	if (balance_args.flags & BTRFS_BALANCE_DATA) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(balance_args, data,
				    btrfs_print_balance_args);
	}
	if (balance_args.flags & BTRFS_BALANCE_METADATA) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(balance_args, meta,
				    btrfs_print_balance_args);
	}
	if (balance_args.flags & BTRFS_BALANCE_SYSTEM) {
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(balance_args, sys,
				    btrfs_print_balance_args);
	}
	tprint_struct_end();
}

static void
btrfs_print_features(const struct btrfs_ioctl_feature_flags *flags)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*flags, compat_flags, btrfs_features_compat,
			  "BTRFS_FEATURE_COMPAT_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*flags, compat_ro_flags,
			  btrfs_features_compat_ro,
			  "BTRFS_FEATURE_COMPAT_RO_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*flags, incompat_flags, btrfs_features_incompat,
			  "BTRFS_FEATURE_INCOMPAT_???");
	tprint_struct_end();
}

static void
btrfs_print_qgroup_limit(const struct btrfs_qgroup_limit *lim)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*lim, flags, btrfs_qgroup_limit_flags,
			  "BTRFS_QGROUP_LIMIT_???");
	tprint_struct_next();
	PRINT_FIELD_U(*lim, max_rfer);
	tprint_struct_next();
	PRINT_FIELD_U(*lim, max_excl);
	tprint_struct_next();
	PRINT_FIELD_U(*lim, rsv_rfer);
	tprint_struct_next();
	PRINT_FIELD_U(*lim, rsv_excl);
	tprint_struct_end();
}

#define btrfs_print_key_type(where_, field_) \
	PRINT_FIELD_XVAL_U((where_), field_, btrfs_key_types, NULL)
#define btrfs_print_objectid(where_, field_) \
	PRINT_FIELD_XVAL_U((where_), field_, btrfs_tree_objectids, NULL)

static void
btrfs_print_data_container_header(const struct btrfs_data_container *container)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*container, bytes_left);
	tprint_struct_next();
	PRINT_FIELD_U(*container, bytes_missing);
	tprint_struct_next();
	PRINT_FIELD_U(*container, elem_cnt);
	tprint_struct_next();
	PRINT_FIELD_U(*container, elem_missed);
}

static void
btrfs_print_data_container_footer(void)
{
	tprint_struct_end();
}

static bool
print_btrfs_data_container_logical_ino(struct tcb *tcp, void *elem_buf,
				       size_t elem_size, void *data)
{
	const struct {
		uint64_t inum;
		uint64_t offset;
		uint64_t root;
	} *const record = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_U(*record, inum);
	tprint_struct_next();
	PRINT_FIELD_U(*record, offset);
	tprint_struct_next();
	PRINT_FIELD_U(*record, root);
	tprint_struct_end();

	return true;
}

static void
btrfs_print_logical_ino_container(struct tcb *tcp,
				  const uint64_t inodes_addr)
{
	struct btrfs_data_container container;

	if (umove_or_printaddr(tcp, inodes_addr, &container))
		return;

	btrfs_print_data_container_header(&container);

	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		const uint64_t val_addr =
			inodes_addr + offsetof(typeof(container), val);
		uint64_t record[3];
		tprint_struct_next();
		tprints_field_name("val");
		print_array(tcp, val_addr, container.elem_cnt / 3,
			    record, sizeof(record),
			    tfetch_mem,
			    print_btrfs_data_container_logical_ino, 0);
	}

	btrfs_print_data_container_footer();
}

static bool
print_btrfs_data_container_ino_path(struct tcb *tcp, void *elem_buf,
				       size_t elem_size, void *data)
{
	const uint64_t *const offset = elem_buf;
	const uint64_t *const val_addr = data;

	printpath(tcp, *val_addr + *offset);

	return true;
}

static void
btrfs_print_ino_path_container(struct tcb *tcp,
			       const uint64_t fspath_addr)
{
	struct btrfs_data_container container;

	if (umove_or_printaddr(tcp, fspath_addr, &container))
		return;

	btrfs_print_data_container_header(&container);

	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		uint64_t val_addr =
			fspath_addr + offsetof(typeof(container), val);
		uint64_t offset;
		tprint_struct_next();
		tprints_field_name("val");
		print_array(tcp, val_addr, container.elem_cnt,
			    &offset, sizeof(offset),
			    tfetch_mem,
			    print_btrfs_data_container_ino_path, &val_addr);
	}

	btrfs_print_data_container_footer();
}

static void
btrfs_print_qgroup_inherit(struct tcb *const tcp, const kernel_ulong_t qgi_addr)
{
	struct btrfs_qgroup_inherit inherit;

	if (umove_or_printaddr(tcp, qgi_addr, &inherit))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(inherit, flags, btrfs_qgroup_inherit_flags,
			  "BTRFS_QGROUP_INHERIT_???");
	tprint_struct_next();
	PRINT_FIELD_U(inherit, num_qgroups);
	tprint_struct_next();
	PRINT_FIELD_U(inherit, num_ref_copies);
	tprint_struct_next();
	PRINT_FIELD_U(inherit, num_excl_copies);

	tprint_struct_next();
	PRINT_FIELD_OBJ_PTR(inherit, lim, btrfs_print_qgroup_limit);

	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		uint64_t record;
		tprint_struct_next();
		tprints_field_name("qgroups");
		print_array(tcp, qgi_addr + offsetof(typeof(inherit), qgroups),
			    inherit.num_qgroups, &record, sizeof(record),
			    tfetch_mem, print_uint_array_member, 0);
	}
	tprint_struct_end();
}

static void
print_btrfs_ioctl_search_key(const struct btrfs_ioctl_search_key *const key,
			     const bool is_entering, const bool is_not_abbrev)
{
	tprint_struct_begin();
	if (is_entering) {
		btrfs_print_objectid(*key, tree_id);

		if (key->min_objectid != BTRFS_FIRST_FREE_OBJECTID ||
		    is_not_abbrev) {
			tprint_struct_next();
			btrfs_print_objectid(*key, min_objectid);
		}

		if (key->max_objectid != BTRFS_LAST_FREE_OBJECTID ||
		    is_not_abbrev) {
			tprint_struct_next();
			btrfs_print_objectid(*key, max_objectid);
		}

		tprint_struct_next();
		PRINT_FIELD_U64(*key, min_offset);
		tprint_struct_next();
		PRINT_FIELD_U64(*key, max_offset);
		tprint_struct_next();
		PRINT_FIELD_U64(*key, min_transid);
		tprint_struct_next();
		PRINT_FIELD_U64(*key, max_transid);

		tprint_struct_next();
		btrfs_print_key_type(*key, min_type);
		tprint_struct_next();
		btrfs_print_key_type(*key, max_type);
		tprint_struct_next();
		PRINT_FIELD_U(*key, nr_items);
	} else {
		PRINT_FIELD_U(*key, nr_items);
	}
	tprint_struct_end();
}

static void
print_btrfs_ioctl_search_header(const struct btrfs_ioctl_search_header *p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, transid);
	tprint_struct_next();
	btrfs_print_objectid(*p, objectid);
	tprint_struct_next();
	PRINT_FIELD_U(*p, offset);
	tprint_struct_next();
	btrfs_print_key_type(*p, type);
	tprint_struct_next();
	PRINT_FIELD_U(*p, len);
	tprint_struct_end();
}

static void
decode_search_arg_buf(struct tcb *tcp, kernel_ulong_t buf_addr, uint64_t buf_size,
		      unsigned int nr_items)
{
	if (entering(tcp))
		return;
	tprint_struct_next();
	if (abbrev(tcp)) {
		tprint_more_data_follows();
	} else {
		tprints_field_name("buf");
		tprint_array_begin();
		uint64_t off = 0;
		for (unsigned int i = 0; i < nr_items; ++i) {
			if (i)
				tprint_array_next();
			struct btrfs_ioctl_search_header sh;
			uint64_t addr = buf_addr + off;
			if (addr < buf_addr || off + sizeof(sh) > buf_size ||
			    i > max_strlen) {
				tprint_more_data_follows();
				break;
			}
			if (!tfetch_mem(tcp, addr, sizeof(sh), &sh)) {
				tprint_more_data_follows();
				printaddr_comment(addr);
				break;
			}
			print_btrfs_ioctl_search_header(&sh);
			off += sizeof(sh) + sh.len;

		}
		tprint_array_end();
	}
}

static bool
print_objectid_callback(struct tcb *tcp, void *elem_buf,
			size_t elem_size, void *data)
{
	printxvals_ex(*(uint64_t *) elem_buf, NULL, XLAT_STYLE_FMT_U,
		      btrfs_tree_objectids, NULL);

	return true;
}

static bool
print_btrfs_ioctl_space_info(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	const struct btrfs_ioctl_space_info *info = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*info, flags, btrfs_space_info_flags,
			  "BTRFS_SPACE_INFO_???");
	tprint_struct_next();
	PRINT_FIELD_U(*info, total_bytes);
	tprint_struct_next();
	PRINT_FIELD_U(*info, used_bytes);
	tprint_struct_end();

	return true;
}

static void
print_btrfs_timespec(const MPERS_PTR_ARG(struct_btrfs_ioctl_timespec *) const arg)
{
	const struct_btrfs_ioctl_timespec *const p = arg;
	tprint_struct_begin();
	PRINT_FIELD_U(*p, sec);
	tprint_struct_next();
	PRINT_FIELD_U(*p, nsec);
	tprint_struct_end();
	tprints_comment(sprinttime_nsec(p->sec, p->nsec));
}

static void
print_btrfs_scrub_progress(const struct btrfs_scrub_progress *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_U(*p, data_extents_scrubbed);
	tprint_struct_next();
	PRINT_FIELD_U(*p, tree_extents_scrubbed);
	tprint_struct_next();
	PRINT_FIELD_U(*p, data_bytes_scrubbed);
	tprint_struct_next();
	PRINT_FIELD_U(*p, tree_bytes_scrubbed);
	tprint_struct_next();
	PRINT_FIELD_U(*p, read_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, csum_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, verify_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, no_csum);
	tprint_struct_next();
	PRINT_FIELD_U(*p, csum_discards);
	tprint_struct_next();
	PRINT_FIELD_U(*p, super_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, malloc_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, uncorrectable_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, corrected_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, last_physical);
	tprint_struct_next();
	PRINT_FIELD_U(*p, unverified_errors);
	tprint_struct_end();
}

static void
print_btrfs_replace_start_params(const typeof_field(struct_btrfs_ioctl_dev_replace_args, start) *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_DEV(*p, srcdevid);
	tprint_struct_next();
	PRINT_FIELD_XVAL(*p, cont_reading_from_srcdev_mode,
			 btrfs_cont_reading_from_srcdev_mode,
			 "BTRFS_IOCTL_DEV_REPLACE_CONT_READING"
			 "_FROM_SRCDEV_MODE_???");
	tprint_struct_next();
	PRINT_FIELD_CSTRING(*p, srcdev_name);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(*p, tgtdev_name);
	tprint_struct_end();
}

static void
print_btrfs_replace_status_params(const typeof_field(struct_btrfs_ioctl_dev_replace_args, status) *const p)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*p, replace_state, btrfs_dev_replace_state,
			 "BTRFS_IOCTL_DEV_REPLACE_STATE_???");

	tprint_struct_next();
	PRINT_FIELD_U(*p, progress_1000);
	if (p->progress_1000 <= 1000)
		tprintf_comment("%u.%u%%",
			(unsigned) p->progress_1000 / 10,
			(unsigned) p->progress_1000 % 10);

	tprint_struct_next();
	PRINT_FIELD_U(*p, time_started);
	tprints_comment(sprinttime(p->time_started));

	tprint_struct_next();
	PRINT_FIELD_U(*p, time_stopped);
	tprints_comment(sprinttime(p->time_stopped));

	tprint_struct_next();
	PRINT_FIELD_U(*p, num_write_errors);
	tprint_struct_next();
	PRINT_FIELD_U(*p, num_uncorrectable_read_errors);
	tprint_struct_end();
}

MPERS_PRINTER_DECL(int, btrfs_ioctl,
		   struct tcb *const tcp, const unsigned int code,
		   const kernel_ulong_t arg)
{
	switch (code) {
	/* Take no arguments; command only. */
	case BTRFS_IOC_TRANS_START:
	case BTRFS_IOC_TRANS_END:
	case BTRFS_IOC_SYNC:
	case BTRFS_IOC_SCRUB_CANCEL:
	case BTRFS_IOC_QUOTA_RESCAN_WAIT:
	/*
	 * The codes for these ioctls are based on each accepting a
	 * vol_args but none of them actually consume an argument.
	 */
	case BTRFS_IOC_DEFRAG:
	case BTRFS_IOC_BALANCE:
		break;

	/* takes a signed int */
	case BTRFS_IOC_BALANCE_CTL:
		tprint_arg_next();
		printxval(btrfs_balance_ctl_cmds, arg, "BTRFS_BALANCE_CTL_???");
		break;

	/* returns a 64 */
	case BTRFS_IOC_START_SYNC: /* R */
		if (entering(tcp))
			return 0;
	ATTRIBUTE_FALLTHROUGH;
	/* takes a u64 */
	case BTRFS_IOC_DEFAULT_SUBVOL: /* W */
	case BTRFS_IOC_WAIT_SYNC: /* W */
		tprint_arg_next();
		printnum_int64(tcp, arg, "%" PRIu64);
		break;

	/* u64 but describe a flags bitfield; we can make that symbolic */
	case BTRFS_IOC_SUBVOL_GETFLAGS: { /* R */
		uint64_t flags;

		if (entering(tcp))
			return 0;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &flags))
			break;

		printflags64(btrfs_snap_flags_v2, flags, "BTRFS_SUBVOL_???");
		break;
	}

	case BTRFS_IOC_SUBVOL_SETFLAGS: { /* W */
		uint64_t flags;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &flags))
			break;

		printflags64(btrfs_snap_flags_v2, flags, "BTRFS_SUBVOL_???");
		break;
	}

	/* More complex types */
	case BTRFS_IOC_BALANCE_V2: /* RW */
		if (entering(tcp)) {
			tprint_arg_next();
			btrfs_print_balance(tcp, arg, false);
			return 0;
		}

		if (syserror(tcp))
			break;

		tprint_value_changed();
		btrfs_print_balance(tcp, arg, true);
		break;
	case BTRFS_IOC_BALANCE_PROGRESS: /* R */
		if (entering(tcp))
			return 0;

		tprint_arg_next();
		btrfs_print_balance(tcp, arg, true);
		break;

	case BTRFS_IOC_DEFRAG_RANGE: { /* W */
		struct btrfs_ioctl_defrag_range_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, start);
		tprint_struct_next();
		PRINT_FIELD_U64(args, len);

		tprint_struct_next();
		PRINT_FIELD_FLAGS(args, flags, btrfs_defrag_flags,
				  "BTRFS_DEFRAG_RANGE_???");
		tprint_struct_next();
		PRINT_FIELD_U(args, extent_thresh);
		tprint_struct_next();
		PRINT_FIELD_XVAL(args, compress_type,
				 btrfs_compress_types, "BTRFS_COMPRESS_???");
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_DEV_INFO: { /* RW */
		struct btrfs_ioctl_dev_info_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_DEV(args, devid);
			if (!IS_ARRAY_ZERO(args.uuid)) {
				tprint_struct_next();
				PRINT_FIELD_UUID(args, uuid);
			}
			tprint_struct_end();
			return 0;
		}

		tprint_struct_begin();

		if (!IS_ARRAY_ZERO(args.uuid)) {
			PRINT_FIELD_UUID(args, uuid);
			tprint_struct_next();
		}

		PRINT_FIELD_U(args, bytes_used);
		tprint_struct_next();
		PRINT_FIELD_U(args, total_bytes);
		tprint_struct_next();
		PRINT_FIELD_CSTRING(args, path);
		tprint_struct_end();

		break;
	}

	case BTRFS_IOC_DEV_REPLACE: { /* RW */
		struct_btrfs_ioctl_dev_replace_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_XVAL(args, cmd, btrfs_dev_replace_cmds,
					 "BTRFS_IOCTL_DEV_REPLACE_CMD_???");
			if (args.cmd == BTRFS_IOCTL_DEV_REPLACE_CMD_START) {
				tprint_struct_next();
				PRINT_FIELD_OBJ_PTR(args, start,
						    print_btrfs_replace_start_params);
			}
			tprint_struct_end();
			return 0;
		}

		tprint_struct_begin();
		PRINT_FIELD_XVAL(args, result, btrfs_dev_replace_results,
				 "BTRFS_IOCTL_DEV_REPLACE_RESULT_???");
		if (args.cmd == BTRFS_IOCTL_DEV_REPLACE_CMD_STATUS) {
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(args, status,
					    print_btrfs_replace_status_params);
		}
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_GET_FEATURES: { /* R */
		struct btrfs_ioctl_feature_flags flags;

		if (entering(tcp))
			return 0;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &flags))
			break;

		btrfs_print_features(&flags);
		break;
	}

	case BTRFS_IOC_SET_FEATURES: { /* W */
		struct btrfs_ioctl_feature_flags flarg[2];

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &flarg))
			break;

		tprint_array_begin();
		btrfs_print_features(&flarg[0]);
		tprint_array_next();
		btrfs_print_features(&flarg[1]);
		tprint_array_end();
		break;
	}

	case BTRFS_IOC_GET_SUPPORTED_FEATURES: { /* R */
		struct btrfs_ioctl_feature_flags flarg[3];

		if (entering(tcp))
			return 0;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &flarg))
			break;

		tprint_array_begin();
		btrfs_print_features(&flarg[0]);
		tprints_comment("supported");

		tprint_array_next();
		btrfs_print_features(&flarg[1]);
		tprints_comment("safe to set");

		tprint_array_next();
		btrfs_print_features(&flarg[2]);
		tprints_comment("safe to clear");
		tprint_array_end();

		break;
	}

	case BTRFS_IOC_FS_INFO: { /* R */
		struct btrfs_ioctl_fs_info_args args;

		if (entering(tcp))
			return 0;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, max_id);
		tprint_struct_next();
		PRINT_FIELD_U(args, num_devices);
		tprint_struct_next();
		PRINT_FIELD_UUID(args, fsid);
		tprint_struct_next();
		PRINT_FIELD_U(args, nodesize);
		tprint_struct_next();
		PRINT_FIELD_U(args, sectorsize);
		tprint_struct_next();
		PRINT_FIELD_U(args, clone_alignment);
		if (args.flags & BTRFS_FS_INFO_FLAG_CSUM_INFO) {
			tprint_struct_next();
			PRINT_FIELD_XVAL(args, csum_type, btrfs_csum_types,
					 "BTRFS_CSUM_TYPE_???");
			tprint_struct_next();
			PRINT_FIELD_U(args, csum_size);
		}
		tprint_struct_next();
		PRINT_FIELD_FLAGS(args, flags, btrfs_fs_info_flags,
				  "BTRFS_FS_INFO_FLAG_???");
		if (args.flags & BTRFS_FS_INFO_FLAG_GENERATION) {
			tprint_struct_next();
			PRINT_FIELD_U(args, generation);
		}
		if (args.flags & BTRFS_FS_INFO_FLAG_METADATA_UUID) {
			tprint_struct_next();
			PRINT_FIELD_UUID(args, metadata_uuid);
		}
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_GET_DEV_STATS: { /* RW */
		struct btrfs_ioctl_get_dev_stats args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();

		if (entering(tcp)) {
			PRINT_FIELD_DEV(args, devid);
			tprint_struct_next();
		}

		PRINT_FIELD_U(args, nr_items);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(args, flags, btrfs_dev_stats_flags,
				  "BTRFS_DEV_STATS_???");

		if (entering(tcp)) {
			tprint_struct_end();
			return 0;
		}

		/*
		 * The structure has a 1k limit; Let's make sure we don't
		 * go off into the middle of nowhere with a bad nr_items
		 * value.
		 */
		tprint_struct_next();
		tprint_array_begin();
		for (uint64_t i = 0; i < args.nr_items; ++i) {
			if (i)
				tprint_array_next();
			if (i >= ARRAY_SIZE(args.values)) {
				tprint_more_data_follows();
				break;
			}

			tprint_array_index_begin();
			printxval_u(btrfs_dev_stats_values, i, NULL);
			tprint_array_index_equal();
			PRINT_VAL_U(args.values[i]);
			tprint_array_index_end();
		}
		tprint_array_end();
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_INO_LOOKUP: { /* RW */
		struct btrfs_ioctl_ino_lookup_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			/* Use subvolume id of the containing root */
			if (args.treeid == 0)
				set_tcb_priv_ulong(tcp, 1);

			tprint_struct_begin();
			btrfs_print_objectid(args, treeid);
			tprint_struct_next();
			btrfs_print_objectid(args, objectid);
			tprint_struct_end();
			return 0;
		}

		tprint_struct_begin();
		if (get_tcb_priv_ulong(tcp)) {
			btrfs_print_objectid(args, treeid);
			tprint_struct_next();
		}

		PRINT_FIELD_CSTRING(args, name);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_INO_PATHS: { /* RW */
		struct btrfs_ioctl_ino_path_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_U(args, inum);
			tprint_struct_next();
			PRINT_FIELD_U(args, size);
			tprint_struct_next();
			PRINT_FIELD_ADDR64(args, fspath);
			tprint_struct_end();
			return 0;
		}

		tprint_struct_begin();
		PRINT_FIELD_OBJ_TCB_VAL(args, fspath, tcp,
					btrfs_print_ino_path_container);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_LOGICAL_INO: { /* RW */
		struct btrfs_ioctl_logical_ino_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_U(args, logical);
			tprint_struct_next();
			PRINT_FIELD_U(args, size);

			if (!IS_ARRAY_ZERO(args.reserved)) {
				tprint_struct_next();
				PRINT_FIELD_X_ARRAY(args, reserved);
			}

			tprint_struct_next();
			PRINT_FIELD_FLAGS(args, flags,
					  btrfs_logical_ino_args_flags,
					  "BTRFS_LOGICAL_INO_ARGS_???");
			tprint_struct_next();
			PRINT_FIELD_ADDR64(args, inodes);
			tprint_struct_end();
			return 0;
		}

		tprint_struct_begin();
		PRINT_FIELD_OBJ_TCB_VAL(args, inodes, tcp,
					btrfs_print_logical_ino_container);

		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_QGROUP_ASSIGN: { /* W */
		struct btrfs_ioctl_qgroup_assign_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, assign);
		tprint_struct_next();
		PRINT_FIELD_U(args, src);
		tprint_struct_next();
		PRINT_FIELD_U(args, dst);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_QGROUP_CREATE: { /* W */
		struct btrfs_ioctl_qgroup_create_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, create);
		tprint_struct_next();
		PRINT_FIELD_U(args, qgroupid);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_QGROUP_LIMIT: { /* R */
		struct btrfs_ioctl_qgroup_limit_args args;

		if (entering(tcp))
			return 0;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, qgroupid);
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(args, lim, btrfs_print_qgroup_limit);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_QUOTA_CTL: { /* W */
		struct btrfs_ioctl_quota_ctl_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_XVAL(args, cmd, btrfs_qgroup_ctl_cmds,
				 "BTRFS_QUOTA_CTL_???");
		tprint_struct_end();

		break;
	}

	case BTRFS_IOC_QUOTA_RESCAN: { /* W */
		struct btrfs_ioctl_quota_rescan_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, flags);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_QUOTA_RESCAN_STATUS: { /* R */
		struct btrfs_ioctl_quota_rescan_args args;

		if (entering(tcp))
			return 0;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_U(args, flags);
		tprint_struct_next();
		btrfs_print_objectid(args, progress);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_SET_RECEIVED_SUBVOL: { /* RW */
		struct_btrfs_ioctl_received_subvol_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_UUID(args, uuid);
			tprint_struct_next();
			PRINT_FIELD_U(args, stransid);
			tprint_struct_next();
			PRINT_FIELD_OBJ_PTR(args, stime,
					    print_btrfs_timespec);
			tprint_struct_next();
			PRINT_FIELD_U(args, flags);
			tprint_struct_end();
			return 0;
		}
		tprint_struct_begin();
		PRINT_FIELD_U(args, rtransid);
		tprint_struct_next();
		PRINT_FIELD_OBJ_PTR(args, rtime, print_btrfs_timespec);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_SCRUB: /* RW */
	case BTRFS_IOC_SCRUB_PROGRESS: { /* RW */
		struct btrfs_ioctl_scrub_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_DEV(args, devid);
			if (code == BTRFS_IOC_SCRUB) {
				tprint_struct_next();
				PRINT_FIELD_U(args, start);
				tprint_struct_next();
				PRINT_FIELD_U64(args, end);
				tprint_struct_next();
				PRINT_FIELD_FLAGS(args, flags,
						  btrfs_scrub_flags,
						  "BTRFS_SCRUB_???");
			}
			tprint_struct_end();
			return 0;
		}
		tprint_struct_begin();
		PRINT_FIELD_OBJ_PTR(args, progress,
				    print_btrfs_scrub_progress);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_TREE_SEARCH: { /* RW */
		struct btrfs_ioctl_search_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args.key))
			break;

		tprint_struct_begin();
		PRINT_FIELD_OBJ_PTR(args, key,
				    print_btrfs_ioctl_search_key,
				    entering(tcp), !abbrev(tcp));
		decode_search_arg_buf(tcp, arg + offsetof(typeof(args), buf),
				      sizeof(args.buf), args.key.nr_items);
		tprint_struct_end();
		if (entering(tcp))
			return 0;
		break;
	}

	case BTRFS_IOC_TREE_SEARCH_V2: { /* RW */
		struct btrfs_ioctl_search_args_v2 args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp)) {
			if (tcp->u_error == EOVERFLOW) {
				tprint_value_changed();
				if (!umove_or_printaddr_ignore_syserror(tcp,
				    arg, &args)) {
					tprint_struct_begin();
					PRINT_FIELD_U(args, buf_size);
					tprint_struct_end();
				}
			}
			break;
		} else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_OBJ_PTR(args, key,
				    print_btrfs_ioctl_search_key,
				    entering(tcp), !abbrev(tcp));
		tprint_struct_next();
		PRINT_FIELD_U(args, buf_size);
		decode_search_arg_buf(tcp, arg + offsetof(typeof(args), buf),
				      args.buf_size, args.key.nr_items);
		tprint_struct_end();
		if (entering(tcp))
			return 0;
		break;
	}

	case BTRFS_IOC_SEND: { /* W */
		struct_btrfs_ioctl_send_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_FD(args, send_fd, tcp);
		tprint_struct_next();
		PRINT_FIELD_U(args, clone_sources_count);

		if (abbrev(tcp)) {
			tprint_struct_next();
			PRINT_FIELD_PTR(args, clone_sources);
		} else {
			tprint_struct_next();
			tprints_field_name("clone_sources");
			uint64_t record;
			print_array(tcp, ptr_to_kulong(args.clone_sources),
				    args.clone_sources_count,
				    &record, sizeof(record),
				    tfetch_mem,
				    print_objectid_callback, 0);
		}
		tprint_struct_next();
		btrfs_print_objectid(args, parent_root);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(args, flags, btrfs_send_flags,
				  "BTRFS_SEND_FLAGS_???");
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_SPACE_INFO: { /* RW */
		struct btrfs_ioctl_space_args args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_U(args, space_slots);
			tprint_struct_end();
			return 0;
		}

		tprint_struct_begin();
		PRINT_FIELD_U(args, total_spaces);

		if (args.space_slots == 0 && args.total_spaces) {
			tprint_struct_end();
			break;
		}

		if (abbrev(tcp)) {
			tprint_struct_next();
			tprint_more_data_follows();
		} else {
			struct btrfs_ioctl_space_info info;
			tprint_struct_next();
			tprints_field_name("spaces");
			print_array(tcp, arg + offsetof(typeof(args), spaces),
				    args.total_spaces,
				    &info, sizeof(info), tfetch_mem,
				    print_btrfs_ioctl_space_info, 0);
		}
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_SNAP_CREATE:
	case BTRFS_IOC_RESIZE:
	case BTRFS_IOC_SCAN_DEV:
	case BTRFS_IOC_FORGET_DEV:
	case BTRFS_IOC_ADD_DEV:
	case BTRFS_IOC_RM_DEV:
	case BTRFS_IOC_SUBVOL_CREATE:
	case BTRFS_IOC_SNAP_DESTROY:
	case BTRFS_IOC_DEVICES_READY: { /* W */
		struct btrfs_ioctl_vol_args args;

		tprint_arg_next();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprint_struct_begin();
		PRINT_FIELD_FD(args, fd, tcp);
		tprint_struct_next();
		PRINT_FIELD_CSTRING(args, name);
		tprint_struct_end();
		break;
	}

	case BTRFS_IOC_SNAP_CREATE_V2:
	case BTRFS_IOC_SUBVOL_CREATE_V2: { /* code is W, but is actually RW */
		struct_btrfs_ioctl_vol_args_v2 args;

		if (entering(tcp))
			tprint_arg_next();
		else if (syserror(tcp))
			break;
		else
			tprint_value_changed();

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			tprint_struct_begin();
			PRINT_FIELD_FD(args, fd, tcp);
			tprint_struct_next();
			PRINT_FIELD_FLAGS(args, flags,
					  btrfs_snap_flags_v2,
					  "BTRFS_SUBVOL_???");
			if (args.flags & BTRFS_SUBVOL_QGROUP_INHERIT) {
				tprint_struct_next();
				PRINT_FIELD_U(args, size);
				tprint_struct_next();
				tprints_field_name("qgroup_inherit");
				btrfs_print_qgroup_inherit(tcp,
					ptr_to_kulong(args.qgroup_inherit));
			}
			tprint_struct_next();
			PRINT_FIELD_CSTRING(args, name);
			tprint_struct_end();
			return 0;
		}
		tprint_struct_begin();
		PRINT_FIELD_U(args, transid);
		tprint_struct_end();
		break;
	}

	default:
		return RVAL_DECODED;
	};
	return RVAL_IOCTL_DECODED;
}
