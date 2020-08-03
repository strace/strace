/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_BTRFS_H

# include DEF_MPERS_TYPE(struct_btrfs_ioctl_dev_replace_args)
# include DEF_MPERS_TYPE(struct_btrfs_ioctl_send_args)
# include DEF_MPERS_TYPE(struct_btrfs_ioctl_received_subvol_args)
# include DEF_MPERS_TYPE(struct_btrfs_ioctl_vol_args_v2)

# include <linux/btrfs.h>

typedef struct btrfs_ioctl_dev_replace_args
	struct_btrfs_ioctl_dev_replace_args;
typedef struct btrfs_ioctl_send_args
	struct_btrfs_ioctl_send_args;
typedef struct btrfs_ioctl_received_subvol_args
	struct_btrfs_ioctl_received_subvol_args;
typedef struct btrfs_ioctl_vol_args_v2
	struct_btrfs_ioctl_vol_args_v2;

#endif /* HAVE_LINUX_BTRFS_H */

#include MPERS_DEFS

#ifdef HAVE_LINUX_BTRFS_H

# include "print_fields.h"
# include "types/btrfs.h"
# include <linux/fs.h>

/*
 * Prior to Linux 3.12, the BTRFS_IOC_DEFAULT_SUBVOL used u64 in
 * its definition, which isn't exported by the kernel.
 */
typedef __u64 u64;

# ifndef HAVE_STRUCT_BTRFS_IOCTL_FEATURE_FLAGS_COMPAT_FLAGS
struct btrfs_ioctl_feature_flags {
	uint64_t compat_flags;
	uint64_t compat_ro_flags;
	uint64_t incompat_flags;
};
# endif

# ifndef HAVE_STRUCT_BTRFS_IOCTL_DEFRAG_RANGE_ARGS_START
struct btrfs_ioctl_defrag_range_args {
	uint64_t start;
	uint64_t len;
	uint64_t flags;
	uint32_t extent_thresh;
	uint32_t compress_type;
	uint32_t unused[4];
};
# endif

# ifndef BTRFS_LABEL_SIZE
#  define BTRFS_LABEL_SIZE 256
# endif

# ifndef BTRFS_IOC_QUOTA_RESCAN
struct btrfs_ioctl_quota_rescan_args {
	uint64_t flags, progress, reserved[6];
};
#  define BTRFS_IOC_QUOTA_RESCAN _IOW(BTRFS_IOCTL_MAGIC, 44, \
					struct btrfs_ioctl_quota_rescan_args)
#  define BTRFS_IOC_QUOTA_RESCAN_STATUS _IOR(BTRFS_IOCTL_MAGIC, 45, \
					struct btrfs_ioctl_quota_rescan_args)
# endif

# ifndef BTRFS_IOC_QUOTA_RESCAN_WAIT
#  define BTRFS_IOC_QUOTA_RESCAN_WAIT _IO(BTRFS_IOCTL_MAGIC, 46)
# endif

# ifndef BTRFS_IOC_GET_FEATURES
#  define BTRFS_IOC_GET_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57, \
					struct btrfs_ioctl_feature_flags)
#  define BTRFS_IOC_SET_FEATURES _IOW(BTRFS_IOCTL_MAGIC, 57, \
					struct btrfs_ioctl_feature_flags[2])
#  define BTRFS_IOC_GET_SUPPORTED_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57, \
					struct btrfs_ioctl_feature_flags[3])
# endif

# ifndef BTRFS_IOC_TREE_SEARCH_V2
#  define BTRFS_IOC_TREE_SEARCH_V2 _IOWR(BTRFS_IOCTL_MAGIC, 17, \
					struct btrfs_ioctl_search_args_v2)
struct btrfs_ioctl_search_args_v2 {
	struct btrfs_ioctl_search_key key; /* in/out - search parameters */
	uint64_t buf_size;		   /* in - size of buffer
					    * out - on EOVERFLOW: needed size
					    *       to store item */
	uint64_t buf[0];		   /* out - found items */
};
# endif

# include "xlat/btrfs_balance_args.h"
# include "xlat/btrfs_balance_ctl_cmds.h"
# include "xlat/btrfs_balance_flags.h"
# include "xlat/btrfs_balance_state.h"
# include "xlat/btrfs_compress_types.h"
# include "xlat/btrfs_cont_reading_from_srcdev_mode.h"
# include "xlat/btrfs_defrag_flags.h"
# include "xlat/btrfs_dev_replace_cmds.h"
# include "xlat/btrfs_dev_replace_results.h"
# include "xlat/btrfs_dev_replace_state.h"
# include "xlat/btrfs_dev_stats_flags.h"
# include "xlat/btrfs_dev_stats_values.h"
# include "xlat/btrfs_features_compat.h"
# include "xlat/btrfs_features_compat_ro.h"
# include "xlat/btrfs_features_incompat.h"
# include "xlat/btrfs_key_types.h"
# include "xlat/btrfs_logical_ino_args_flags.h"
# include "xlat/btrfs_qgroup_ctl_cmds.h"
# include "xlat/btrfs_qgroup_inherit_flags.h"
# include "xlat/btrfs_qgroup_limit_flags.h"
# include "xlat/btrfs_qgroup_status_flags.h"
# include "xlat/btrfs_scrub_flags.h"
# include "xlat/btrfs_send_flags.h"
# include "xlat/btrfs_snap_flags_v2.h"
# include "xlat/btrfs_space_info_flags.h"
# include "xlat/btrfs_tree_objectids.h"

static void
btrfs_print_balance_args(const char *name, const struct btrfs_balance_args *bba)
{
	tprintf(", %s=", name);
	PRINT_FIELD_FLAGS("{", *bba, profiles, btrfs_space_info_flags,
			  "BTRFS_BLOCK_GROUP_???");
	PRINT_FIELD_U64(", ", *bba, usage);
	PRINT_FIELD_DEV(", ", *bba, devid);
	PRINT_FIELD_U64(", ", *bba, pstart);
	PRINT_FIELD_U64(", ", *bba, pend);
	PRINT_FIELD_U64(", ", *bba, vstart);
	PRINT_FIELD_U64(", ", *bba, vend);
	PRINT_FIELD_U64(", ", *bba, target);
	PRINT_FIELD_FLAGS(", ", *bba, flags, btrfs_balance_args,
			  "BTRFS_BALANCE_ARGS_???");
	tprints("}");
}

static void
btrfs_print_balance(struct tcb *const tcp, const kernel_ulong_t arg, bool out)
{
	struct btrfs_ioctl_balance_args balance_args;

	if (umove_or_printaddr(tcp, arg, &balance_args))
		return;

	PRINT_FIELD_FLAGS("{", balance_args, flags, btrfs_balance_flags,
			  "BTRFS_BALANCE_???");
	if (out)
		PRINT_FIELD_FLAGS(", ", balance_args, state,
				  btrfs_balance_state,
				  "BTRFS_BALANCE_STATE_???");

	if (balance_args.flags & BTRFS_BALANCE_DATA)
		btrfs_print_balance_args("data", &balance_args.data);
	if (balance_args.flags & BTRFS_BALANCE_METADATA)
		btrfs_print_balance_args("meta", &balance_args.meta);
	if (balance_args.flags & BTRFS_BALANCE_SYSTEM)
		btrfs_print_balance_args("sys", &balance_args.sys);
	tprints("}");
}

static void
btrfs_print_features(const struct btrfs_ioctl_feature_flags *flags)
{
	PRINT_FIELD_FLAGS("{", *flags, compat_flags, btrfs_features_compat,
			  "BTRFS_FEATURE_COMPAT_???");
	PRINT_FIELD_FLAGS(", ", *flags, compat_ro_flags,
			  btrfs_features_compat_ro,
			  "BTRFS_FEATURE_COMPAT_RO_???");
	PRINT_FIELD_FLAGS(", ", *flags, incompat_flags, btrfs_features_incompat,
			  "BTRFS_FEATURE_INCOMPAT_???");
	tprints("}");
}

static void
btrfs_print_qgroup_limit(const struct btrfs_qgroup_limit *lim)
{
	PRINT_FIELD_FLAGS(", lim={", *lim, flags, btrfs_qgroup_limit_flags,
			  "BTRFS_QGROUP_LIMIT_???");
	PRINT_FIELD_U(", ", *lim, max_rfer);
	PRINT_FIELD_U(", ", *lim, max_excl);
	PRINT_FIELD_U(", ", *lim, rsv_rfer);
	PRINT_FIELD_U(", ", *lim, rsv_excl);
	tprints("}");
}

# define btrfs_print_key_type(prefix_, where_, field_) \
	PRINT_FIELD_XVAL_U((prefix_), (where_), field_, btrfs_key_types, NULL)
# define btrfs_print_objectid(prefix_, where_, field_) \
	PRINT_FIELD_XVAL_U((prefix_), (where_), field_, btrfs_tree_objectids, \
			   NULL)

static void
btrfs_print_data_container_header(const struct btrfs_data_container *container)
{
	PRINT_FIELD_U("{", *container, bytes_left);
	PRINT_FIELD_U(", ", *container, bytes_missing);
	PRINT_FIELD_U(", ", *container, elem_cnt);
	PRINT_FIELD_U(", ", *container, elem_missed);
}

static void
btrfs_print_data_container_footer(void)
{
	tprints("}");
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

	PRINT_FIELD_U("{", *record, inum);
	PRINT_FIELD_U(", ", *record, offset);
	PRINT_FIELD_U(", ", *record, root);
	tprints("}");

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
		tprints(", ...");
	} else {
		const uint64_t val_addr =
			inodes_addr + offsetof(typeof(container), val);
		uint64_t record[3];
		tprints(", val=");
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
		tprints(", ...");
	} else {
		uint64_t val_addr =
			fspath_addr + offsetof(typeof(container), val);
		uint64_t offset;
		tprints(", val=");
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

	PRINT_FIELD_FLAGS("{", inherit, flags, btrfs_qgroup_inherit_flags,
			  "BTRFS_QGROUP_INHERIT_???");
	PRINT_FIELD_U(", ", inherit, num_qgroups);
	PRINT_FIELD_U(", ", inherit, num_ref_copies);
	PRINT_FIELD_U(", ", inherit, num_excl_copies);

	btrfs_print_qgroup_limit(&inherit.lim);

	if (abbrev(tcp)) {
		tprints(", ...");
	} else {
		uint64_t record;
		tprints(", qgroups=");
		print_array(tcp, qgi_addr + offsetof(typeof(inherit), qgroups),
			    inherit.num_qgroups, &record, sizeof(record),
			    tfetch_mem, print_uint64_array_member, 0);
	}
	tprints("}");
}

static void
btrfs_print_tree_search(struct tcb *tcp, struct btrfs_ioctl_search_key *key,
			uint64_t buf_addr, uint64_t buf_size, bool print_size)
{
	if (entering(tcp)) {
		btrfs_print_objectid("{key={", *key, tree_id);

		if (key->min_objectid != BTRFS_FIRST_FREE_OBJECTID ||
		    !abbrev(tcp))
			btrfs_print_objectid(", ", *key, min_objectid);

		if (key->max_objectid != BTRFS_LAST_FREE_OBJECTID ||
		    !abbrev(tcp))
			btrfs_print_objectid(", ", *key, max_objectid);

		PRINT_FIELD_U64(", ", *key, min_offset);
		PRINT_FIELD_U64(", ", *key, max_offset);
		PRINT_FIELD_U64(", ", *key, min_transid);
		PRINT_FIELD_U64(", ", *key, max_transid);

		btrfs_print_key_type(", ", *key, min_type);
		btrfs_print_key_type(", ", *key, max_type);
		PRINT_FIELD_U(", ", *key, nr_items);
		tprints("}");
		if (print_size)
			tprintf(", buf_size=%" PRIu64, buf_size);
		tprints("}");
	} else {
		PRINT_FIELD_U("{key={", *key, nr_items);
		tprints("}");
		if (print_size)
			tprintf(", buf_size=%" PRIu64, buf_size);
		if (abbrev(tcp)) {
			tprints(", ...");
		} else {
			uint64_t i;
			uint64_t off = 0;
			tprints(", buf=[");
			for (i = 0; i < key->nr_items; i++) {
				struct btrfs_ioctl_search_header sh;
				uint64_t addr = buf_addr + off;
				if (i)
					tprints(", ");
				if (i > max_strlen) {
					tprints("...");
					break;
				}
				if (umove(tcp, addr, &sh)) {
					tprints("...");
					printaddr_comment(addr);
					break;
				}
				PRINT_FIELD_U("{", sh, transid);
				btrfs_print_objectid(", ", sh, objectid);
				PRINT_FIELD_U(", ", sh, offset);
				btrfs_print_key_type(", ", sh, type);
				PRINT_FIELD_U(", ", sh, len);
				tprints("}");
				off += sizeof(sh) + sh.len;

			}
			tprints("]");
		}
		tprints("}");
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

	PRINT_FIELD_FLAGS("{", *info, flags, btrfs_space_info_flags,
			  "BTRFS_SPACE_INFO_???");
	PRINT_FIELD_U(", ", *info, total_bytes);
	PRINT_FIELD_U(", ", *info, used_bytes);
	tprints("}");

	return true;
}

static void
print_btrfs_timespec(const char *prefix, uint64_t sec, uint32_t nsec)
{
	tprintf("%s{sec=%" PRIu64 ", nsec=%u}", prefix, sec, nsec);
	tprints_comment(sprinttime_nsec(sec, nsec));
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
		tprints(", ");
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
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIu64);
		break;

	/* u64 but describe a flags bitfield; we can make that symbolic */
	case BTRFS_IOC_SUBVOL_GETFLAGS: { /* R */
		uint64_t flags;

		if (entering(tcp))
			return 0;

		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &flags))
			break;

		printflags64(btrfs_snap_flags_v2, flags, "BTRFS_SUBVOL_???");
		break;
	}

	case BTRFS_IOC_SUBVOL_SETFLAGS: { /* W */
		uint64_t flags;

		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &flags))
			break;

		printflags64(btrfs_snap_flags_v2, flags, "BTRFS_SUBVOL_???");
		break;
	}

	/* More complex types */
	case BTRFS_IOC_BALANCE_V2: /* RW */
		if (entering(tcp)) {
			tprints(", ");
			btrfs_print_balance(tcp, arg, false);
			return 0;
		}

		if (syserror(tcp))
			break;

		tprints(" => ");
		btrfs_print_balance(tcp, arg, true);
		break;
	case BTRFS_IOC_BALANCE_PROGRESS: /* R */
		if (entering(tcp))
			return 0;

		tprints(", ");
		btrfs_print_balance(tcp, arg, true);
		break;

	case BTRFS_IOC_DEFRAG_RANGE: { /* W */
		struct btrfs_ioctl_defrag_range_args args;

		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_U("{", args, start);
		PRINT_FIELD_U64(", ", args, len);

		PRINT_FIELD_FLAGS(", ", args, flags, btrfs_defrag_flags,
				  "BTRFS_DEFRAG_RANGE_???");
		PRINT_FIELD_U(", ", args, extent_thresh);
		PRINT_FIELD_XVAL(", ", args, compress_type,
				 btrfs_compress_types, "BTRFS_COMPRESS_???");
		tprints("}");
		break;
	}

	case BTRFS_IOC_DEV_INFO: { /* RW */
		struct btrfs_ioctl_dev_info_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_DEV("{", args, devid);
			if (!IS_ARRAY_ZERO(args.uuid))
				PRINT_FIELD_UUID(", ", args, uuid);
			tprints("}");
			return 0;
		}

		tprints("{");

		if (!IS_ARRAY_ZERO(args.uuid)) {
			PRINT_FIELD_UUID("", args, uuid);
			tprints(", ");
		}

		PRINT_FIELD_U("", args, bytes_used);
		PRINT_FIELD_U(", ", args, total_bytes);
		PRINT_FIELD_CSTRING(", ", args, path);
		tprints("}");

		break;
	}

	case BTRFS_IOC_DEV_REPLACE: { /* RW */
		struct_btrfs_ioctl_dev_replace_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_XVAL("{", args, cmd, btrfs_dev_replace_cmds,
					 "BTRFS_IOCTL_DEV_REPLACE_CMD_???");
			if (args.cmd == BTRFS_IOCTL_DEV_REPLACE_CMD_START) {
				PRINT_FIELD_DEV(", start={", args.start,
					      srcdevid);
				PRINT_FIELD_XVAL(", ", args.start,
					cont_reading_from_srcdev_mode,
					btrfs_cont_reading_from_srcdev_mode,
					"BTRFS_IOCTL_DEV_REPLACE_CONT_READING"
					"_FROM_SRCDEV_MODE_???");
				PRINT_FIELD_CSTRING(", ", args.start,
						    srcdev_name);
				PRINT_FIELD_CSTRING(", ", args.start,
						    tgtdev_name);
				tprints("}");

			}
			tprints("}");
			return 0;
		}

		PRINT_FIELD_XVAL("{", args, result, btrfs_dev_replace_results,
				 "BTRFS_IOCTL_DEV_REPLACE_RESULT_???");
		if (args.cmd == BTRFS_IOCTL_DEV_REPLACE_CMD_STATUS) {
			PRINT_FIELD_XVAL(", status={", args.status,
					 replace_state, btrfs_dev_replace_state,
					 "BTRFS_IOCTL_DEV_REPLACE_STATE_???");

			PRINT_FIELD_U(", ", args.status, progress_1000);
			if (args.status.progress_1000 <= 1000)
				tprintf_comment("%u.%u%%",
					(unsigned) args.status.progress_1000 / 10,
					(unsigned) args.status.progress_1000 % 10);

			PRINT_FIELD_U(", ", args.status, time_started);
			tprints_comment(sprinttime(args.status.time_started));

			PRINT_FIELD_U(", ", args.status, time_stopped);
			tprints_comment(sprinttime(args.status.time_stopped));

			PRINT_FIELD_U(", ", args.status, num_write_errors);
			PRINT_FIELD_U(", ", args.status,
				      num_uncorrectable_read_errors);
		}
		tprints("}");
		break;
	}

	case BTRFS_IOC_GET_FEATURES: { /* R */
		struct btrfs_ioctl_feature_flags flags;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &flags))
			break;

		btrfs_print_features(&flags);
		break;
	}

	case BTRFS_IOC_SET_FEATURES: { /* W */
		struct btrfs_ioctl_feature_flags flarg[2];

		tprints(", ");

		if (umove_or_printaddr(tcp, arg, &flarg))
			break;

		tprints("[");
		btrfs_print_features(&flarg[0]);
		tprints(", ");
		btrfs_print_features(&flarg[1]);
		tprints("]");
		break;
	}

	case BTRFS_IOC_GET_SUPPORTED_FEATURES: { /* R */
		struct btrfs_ioctl_feature_flags flarg[3];

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &flarg))
			break;

		tprints("[");
		btrfs_print_features(&flarg[0]);
		tprints_comment("supported");

		tprints(", ");
		btrfs_print_features(&flarg[1]);
		tprints_comment("safe to set");

		tprints(", ");
		btrfs_print_features(&flarg[2]);
		tprints_comment("safe to clear");
		tprints("]");

		break;
	}

	case BTRFS_IOC_FS_INFO: { /* R */
		struct btrfs_ioctl_fs_info_args args;
		uint32_t nodesize, sectorsize, clone_alignment;
# ifndef HAVE_STRUCT_BTRFS_IOCTL_FS_INFO_ARGS_NODESIZE
		uint32_t *reserved32;
# endif

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

# ifdef HAVE_STRUCT_BTRFS_IOCTL_FS_INFO_ARGS_NODESIZE
		nodesize = args.nodesize,
		sectorsize = args.sectorsize,
		clone_alignment = args.clone_alignment;
# else
		reserved32 = (void *) args.reserved;
		nodesize = reserved32[0];
		sectorsize = reserved32[1];
		clone_alignment = reserved32[2];
# endif
		PRINT_FIELD_U("{", args, max_id);
		PRINT_FIELD_U(", ", args, num_devices);
		PRINT_FIELD_UUID(", ", args, fsid);
		tprintf(", nodesize=%u, sectorsize=%u, clone_alignment=%u",
			nodesize, sectorsize, clone_alignment);
		tprints("}");
		break;
	}

	case BTRFS_IOC_GET_DEV_STATS: { /* RW */
		struct btrfs_ioctl_get_dev_stats args;
		uint64_t i;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprints("{");

		if (entering(tcp)) {
			PRINT_FIELD_DEV("", args, devid);
			tprints(", ");
		}

		PRINT_FIELD_U("", args, nr_items);
		PRINT_FIELD_FLAGS(", ", args, flags, btrfs_dev_stats_flags,
				  "BTRFS_DEV_STATS_???");

		if (entering(tcp)) {
			tprints("}");
			return 0;
		}

		/*
		 * The structure has a 1k limit; Let's make sure we don't
		 * go off into the middle of nowhere with a bad nr_items
		 * value.
		 */
		tprints(", [");
		for (i = 0; i < args.nr_items; i++) {
			if (i)
				tprints(", ");
			if (i >= ARRAY_SIZE(args.values)) {
				tprints("...");
				break;
			}

			tprints("[");
			printxval_u(btrfs_dev_stats_values, i, NULL);
			tprintf("] = %" PRI__u64, args.values[i]);
		}
		tprints("]}");
		break;
	}

	case BTRFS_IOC_INO_LOOKUP: { /* RW */
		struct btrfs_ioctl_ino_lookup_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			/* Use subvolume id of the containing root */
			if (args.treeid == 0)
				set_tcb_priv_ulong(tcp, 1);

			btrfs_print_objectid("{", args, treeid);
			btrfs_print_objectid(", ", args, objectid);
			tprints("}");
			return 0;
		}

		tprints("{");
		if (get_tcb_priv_ulong(tcp)) {
			btrfs_print_objectid("", args, treeid);
			tprints(", ");
		}

		PRINT_FIELD_CSTRING("", args, name);
		tprints("}");
		break;
	}

	case BTRFS_IOC_INO_PATHS: { /* RW */
		struct btrfs_ioctl_ino_path_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_U("{", args, inum);
			PRINT_FIELD_U(", ", args, size);
			PRINT_FIELD_ADDR64(", ", args, fspath);
			tprints("}");
			return 0;
		}

		tprints("{fspath=");
		btrfs_print_ino_path_container(tcp, args.fspath);

		tprints("}");
		break;
	}

	case BTRFS_IOC_LOGICAL_INO: { /* RW */
		struct_btrfs_ioctl_logical_ino_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_U("{", args, logical);
			PRINT_FIELD_U(", ", args, size);

			if (!IS_ARRAY_ZERO(args.reserved))
				PRINT_FIELD_X_ARRAY(", ", args, reserved);

			tprints(", flags=");
			printflags64(btrfs_logical_ino_args_flags, args.flags,
				     "BTRFS_LOGICAL_INO_ARGS_???");
			PRINT_FIELD_ADDR64(", ", args, inodes);
			tprints("}");
			return 0;
		}

		tprints("{inodes=");
		btrfs_print_logical_ino_container(tcp, args.inodes);

		tprints("}");
		break;
	}

	case BTRFS_IOC_QGROUP_ASSIGN: { /* W */
		struct btrfs_ioctl_qgroup_assign_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_U("{", args, assign);
		PRINT_FIELD_U(", ", args, src);
		PRINT_FIELD_U(", ", args, dst);
		tprints("}");
		break;
	}

	case BTRFS_IOC_QGROUP_CREATE: { /* W */
		struct btrfs_ioctl_qgroup_create_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_U("{", args, create);
		PRINT_FIELD_U(", ", args, qgroupid);
		tprints("}");
		break;
	}

	case BTRFS_IOC_QGROUP_LIMIT: { /* R */
		struct btrfs_ioctl_qgroup_limit_args args;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_U("{", args, qgroupid);
		btrfs_print_qgroup_limit(&args.lim);
		tprints("}");
		break;
	}

	case BTRFS_IOC_QUOTA_CTL: { /* W */
		struct btrfs_ioctl_quota_ctl_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_XVAL("{", args, cmd, btrfs_qgroup_ctl_cmds,
				 "BTRFS_QUOTA_CTL_???");
		tprints("}");

		break;
	}

	case BTRFS_IOC_QUOTA_RESCAN: { /* W */
		struct btrfs_ioctl_quota_rescan_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_U("{", args, flags);
		tprints("}");
		break;
	}

	case BTRFS_IOC_QUOTA_RESCAN_STATUS: { /* R */
		struct btrfs_ioctl_quota_rescan_args args;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_U("{", args, flags);
		btrfs_print_objectid(", ", args, progress);
		tprints("}");
		break;
	}

	case BTRFS_IOC_SET_RECEIVED_SUBVOL: { /* RW */
		struct_btrfs_ioctl_received_subvol_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_UUID("{", args, uuid);
			PRINT_FIELD_U(", ", args, stransid);
			print_btrfs_timespec(", stime=",
					     args.stime.sec, args.stime.nsec);
			PRINT_FIELD_U(", ", args, flags);
			tprints("}");
			return 0;
		}
		PRINT_FIELD_U("{", args, rtransid);
		print_btrfs_timespec(", rtime=",
				     args.rtime.sec, args.rtime.nsec);
		tprints("}");
		break;
	}

	case BTRFS_IOC_SCRUB: /* RW */
	case BTRFS_IOC_SCRUB_PROGRESS: { /* RW */
		struct btrfs_ioctl_scrub_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_DEV("{", args, devid);
			if (code == BTRFS_IOC_SCRUB) {
				PRINT_FIELD_U(", ", args, start);
				PRINT_FIELD_U64(", ", args, end);
				PRINT_FIELD_FLAGS(", ", args, flags,
						  btrfs_scrub_flags,
						  "BTRFS_SCRUB_???");
			}
			tprints("}");
			return 0;
		}
		PRINT_FIELD_U("{progress={", args.progress,
			      data_extents_scrubbed);
		PRINT_FIELD_U(", ", args.progress, tree_extents_scrubbed);
		PRINT_FIELD_U(", ", args.progress, data_bytes_scrubbed);
		PRINT_FIELD_U(", ", args.progress, tree_bytes_scrubbed);
		PRINT_FIELD_U(", ", args.progress, read_errors);
		PRINT_FIELD_U(", ", args.progress, csum_errors);
		PRINT_FIELD_U(", ", args.progress, verify_errors);
		PRINT_FIELD_U(", ", args.progress, no_csum);
		PRINT_FIELD_U(", ", args.progress, csum_discards);
		PRINT_FIELD_U(", ", args.progress, super_errors);
		PRINT_FIELD_U(", ", args.progress, malloc_errors);
		PRINT_FIELD_U(", ", args.progress, uncorrectable_errors);
		PRINT_FIELD_U(", ", args.progress, corrected_errors);
		PRINT_FIELD_U(", ", args.progress, last_physical);
		PRINT_FIELD_U(", ", args.progress, unverified_errors);
		tprints("}}");
		break;
	}

	case BTRFS_IOC_TREE_SEARCH: { /* RW */
		struct btrfs_ioctl_search_args args;
		uint64_t buf_offset;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		buf_offset = offsetof(struct btrfs_ioctl_search_args, buf);
		btrfs_print_tree_search(tcp, &args.key, arg + buf_offset,
					sizeof(args.buf), false);
		if (entering(tcp))
			return 0;
		break;
	}

	case BTRFS_IOC_TREE_SEARCH_V2: { /* RW */
		struct btrfs_ioctl_search_args_v2 args;
		uint64_t buf_offset;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp)) {
			if (tcp->u_error == EOVERFLOW) {
				tprints(" => ");
				if (!umove_or_printaddr_ignore_syserror(tcp,
				    arg, &args)) {
					PRINT_FIELD_U("{", args, buf_size);
					tprints("}");
				}
			}
			break;
		} else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		buf_offset = offsetof(struct btrfs_ioctl_search_args_v2, buf);
		btrfs_print_tree_search(tcp, &args.key, arg + buf_offset,
					args.buf_size, true);
		if (entering(tcp))
			return 0;
		break;
	}

	case BTRFS_IOC_SEND: { /* W */
		struct_btrfs_ioctl_send_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_FD("{", args, send_fd, tcp);
		PRINT_FIELD_U(", ", args, clone_sources_count);

		tprints(", clone_sources=");
		if (abbrev(tcp))
			printaddr((uintptr_t) args.clone_sources);
		else {
			uint64_t record;
			print_array(tcp, ptr_to_kulong(args.clone_sources),
				    args.clone_sources_count,
				    &record, sizeof(record),
				    tfetch_mem,
				    print_objectid_callback, 0);
		}
		btrfs_print_objectid(", ", args, parent_root);
		PRINT_FIELD_FLAGS(", ", args, flags, btrfs_send_flags,
				  "BTRFS_SEND_FLAGS_???");
		tprints("}");
		break;
	}

	case BTRFS_IOC_SPACE_INFO: { /* RW */
		struct btrfs_ioctl_space_args args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_U("{", args, space_slots);
			tprints("}");
			return 0;
		}

		PRINT_FIELD_U("{", args, total_spaces);

		if (args.space_slots == 0 && args.total_spaces) {
			tprints("}");
			break;
		}

		if (abbrev(tcp)) {
			tprints(", ...");
		} else {
			struct btrfs_ioctl_space_info info;
			tprints(", spaces=");
			print_array(tcp, arg + offsetof(typeof(args), spaces),
				    args.total_spaces,
				    &info, sizeof(info), tfetch_mem,
				    print_btrfs_ioctl_space_info, 0);
		}
		tprints("}");
		break;
	}

	case BTRFS_IOC_SNAP_CREATE:
	case BTRFS_IOC_RESIZE:
	case BTRFS_IOC_SCAN_DEV:
# ifdef BTRFS_IOC_FORGET_DEV
	case BTRFS_IOC_FORGET_DEV:
# endif
	case BTRFS_IOC_ADD_DEV:
	case BTRFS_IOC_RM_DEV:
	case BTRFS_IOC_SUBVOL_CREATE:
	case BTRFS_IOC_SNAP_DESTROY:
	case BTRFS_IOC_DEVICES_READY: { /* W */
		struct btrfs_ioctl_vol_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_FD("{", args, fd, tcp);
		PRINT_FIELD_CSTRING(", ", args, name);
		tprints("}");
		break;
	}

	case BTRFS_IOC_SNAP_CREATE_V2:
	case BTRFS_IOC_SUBVOL_CREATE_V2: { /* code is W, but is actually RW */
		struct_btrfs_ioctl_vol_args_v2 args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_FD("{", args, fd, tcp);
			PRINT_FIELD_FLAGS(", ", args, flags,
					  btrfs_snap_flags_v2,
					  "BTRFS_SUBVOL_???");
			if (args.flags & BTRFS_SUBVOL_QGROUP_INHERIT) {
				PRINT_FIELD_U(", ", args, size);
				tprints(", qgroup_inherit=");
				btrfs_print_qgroup_inherit(tcp,
					ptr_to_kulong(args.qgroup_inherit));
			}
			PRINT_FIELD_CSTRING(", ", args, name);
			tprints("}");
			return 0;
		}
		PRINT_FIELD_U("{", args, transid);
		tprints("}");
		break;
	}

	case BTRFS_IOC_GET_FSLABEL: /* R */
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case BTRFS_IOC_SET_FSLABEL: { /* W */
		char label[BTRFS_LABEL_SIZE];

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &label))
			break;
		print_quoted_cstring(label, sizeof(label));
		break;
	}

	case BTRFS_IOC_CLONE:			/* FICLONE */
	case BTRFS_IOC_CLONE_RANGE:		/* FICLONERANGE */
# ifdef BTRFS_IOC_FILE_EXTENT_SAME
	case BTRFS_IOC_FILE_EXTENT_SAME:	/* FIDEDUPERANGE */
# endif
		/*
		 * FICLONE, FICLONERANGE, and FIDEDUPERANGE started out as
		 * btrfs ioctls and the code was kept for the generic
		 * implementations.  We use the BTRFS_* names here because
		 * they will be available on older systems.
		 */
		return file_ioctl(tcp, code, arg);

	default:
		return RVAL_DECODED;
	};
	return RVAL_IOCTL_DECODED;
}
#endif /* HAVE_LINUX_BTRFS_H */
