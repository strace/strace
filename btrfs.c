/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#ifdef HAVE_LINUX_BTRFS_H

#include DEF_MPERS_TYPE(struct_btrfs_ioctl_dev_replace_args)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_send_args)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_received_subvol_args)
#include DEF_MPERS_TYPE(struct_btrfs_ioctl_vol_args_v2)

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

#include <linux/fs.h>

/*
 * Prior to Linux 3.12, the BTRFS_IOC_DEFAULT_SUBVOL used u64 in
 * its definition, which isn't exported by the kernel.
 */
typedef __u64 u64;

#ifndef HAVE_STRUCT_BTRFS_IOCTL_FEATURE_FLAGS_COMPAT_FLAGS
struct btrfs_ioctl_feature_flags {
	uint64_t compat_flags;
	uint64_t compat_ro_flags;
	uint64_t incompat_flags;
};
#endif

#ifndef HAVE_STRUCT_BTRFS_IOCTL_DEFRAG_RANGE_ARGS_START
struct btrfs_ioctl_defrag_range_args {
	uint64_t start;
	uint64_t len;
	uint64_t flags;
	uint32_t extent_thresh;
	uint32_t compress_type;
	uint32_t unused[4];
};
#endif

#ifndef BTRFS_LABEL_SIZE
# define BTRFS_LABEL_SIZE 256
#endif

#ifndef BTRFS_FIRST_FREE_OBJECTID
# define BTRFS_FIRST_FREE_OBJECTID 256ULL
#endif

#ifndef BTRFS_IOC_QUOTA_RESCAN
struct btrfs_ioctl_quota_rescan_args {
	uint64_t flags, progress, reserved[6];
};
# define BTRFS_IOC_QUOTA_RESCAN _IOW(BTRFS_IOCTL_MAGIC, 44, \
					struct btrfs_ioctl_quota_rescan_args)
# define BTRFS_IOC_QUOTA_RESCAN_STATUS _IOR(BTRFS_IOCTL_MAGIC, 45, \
					struct btrfs_ioctl_quota_rescan_args)
#endif

#ifndef BTRFS_IOC_QUOTA_RESCAN_WAIT
# define BTRFS_IOC_QUOTA_RESCAN_WAIT _IO(BTRFS_IOCTL_MAGIC, 46)
#endif

#ifndef BTRFS_IOC_GET_FEATURES
# define BTRFS_IOC_GET_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57, \
					struct btrfs_ioctl_feature_flags)
# define BTRFS_IOC_SET_FEATURES _IOW(BTRFS_IOCTL_MAGIC, 57, \
					struct btrfs_ioctl_feature_flags[2])
# define BTRFS_IOC_GET_SUPPORTED_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57, \
					struct btrfs_ioctl_feature_flags[3])
#endif

#ifndef BTRFS_IOC_TREE_SEARCH_V2
# define BTRFS_IOC_TREE_SEARCH_V2 _IOWR(BTRFS_IOCTL_MAGIC, 17, \
					struct btrfs_ioctl_search_args_v2)
struct btrfs_ioctl_search_args_v2 {
	struct btrfs_ioctl_search_key key; /* in/out - search parameters */
	uint64_t buf_size;		   /* in - size of buffer
					    * out - on EOVERFLOW: needed size
					    *       to store item */
	uint64_t buf[0];		   /* out - found items */
};
#endif

#include "xlat/btrfs_balance_args.h"
#include "xlat/btrfs_balance_ctl_cmds.h"
#include "xlat/btrfs_balance_flags.h"
#include "xlat/btrfs_balance_state.h"
#include "xlat/btrfs_compress_types.h"
#include "xlat/btrfs_defrag_flags.h"
#include "xlat/btrfs_dev_replace_cmds.h"
#include "xlat/btrfs_dev_replace_results.h"
#include "xlat/btrfs_dev_replace_state.h"
#include "xlat/btrfs_dev_stats_flags.h"
#include "xlat/btrfs_dev_stats_values.h"
#include "xlat/btrfs_features_compat.h"
#include "xlat/btrfs_features_compat_ro.h"
#include "xlat/btrfs_features_incompat.h"
#include "xlat/btrfs_key_types.h"
#include "xlat/btrfs_qgroup_ctl_cmds.h"
#include "xlat/btrfs_qgroup_inherit_flags.h"
#include "xlat/btrfs_qgroup_limit_flags.h"
#include "xlat/btrfs_qgroup_status_flags.h"
#include "xlat/btrfs_scrub_flags.h"
#include "xlat/btrfs_send_flags.h"
#include "xlat/btrfs_snap_flags_v2.h"
#include "xlat/btrfs_space_info_flags.h"
#include "xlat/btrfs_tree_objectids.h"

static inline char
prnibble(char v)
{
	if (v >= 10)
		return 'a' + (v - 10);
	return '0' + v;
}

/* 8-4-4-4-12 = 36 characters */
#define UUID_STRING_SIZE 36

/* Formats uuid, returns 0 if it's all zeroes */
static int
btrfs_unparse_uuid(unsigned char *uuid, char *out)
{
	int i;
	int ret = 0;
	for (i = 0; i < BTRFS_UUID_SIZE; i++) {
		if (i == 4 || i == 6 || i == 8 || i == 10)
			*out++ = '-';
		*out++ = prnibble(uuid[i] >> 4);
		*out++ = prnibble(uuid[i] & 0xf);
		if (uuid[i])
			ret = 1;
	}
	*out = '\0';
	return ret;
}

static void
print_u64(const char *name, uint64_t value)
{
	tprintf(", %s=%" PRIu64, name, value);
	if (value == UINT64_MAX)
		tprints_comment("UINT64_MAX");
}

#define print_member_u64(obj, name) print_u64(#name, obj->name)

static void
btrfs_print_balance_args(const char *name, const struct btrfs_balance_args *bba)
{
	tprintf(", %s={profiles=", name);
	printflags64(btrfs_space_info_flags, bba->profiles,
		     "BTRFS_BLOCK_GROUP_???");
	print_member_u64(bba, usage);
	print_member_u64(bba, devid);
	print_member_u64(bba, pstart);
	print_member_u64(bba, pend);
	print_member_u64(bba, vstart);
	print_member_u64(bba, vend);
	print_member_u64(bba, target);
	tprints(", flags=");
	printflags64(btrfs_balance_args, bba->flags, "BTRFS_BALANCE_ARGS_???");
	tprints("}");
}

static void
btrfs_print_balance(struct tcb *const tcp, const kernel_ulong_t arg, bool out)
{
	struct btrfs_ioctl_balance_args balance_args;

	if (umove_or_printaddr(tcp, arg, &balance_args))
		return;

	tprints("{flags=");
	printflags64(btrfs_balance_flags, balance_args.flags,
		     "BTRFS_BALANCE_???");
	if (out) {
		tprints(", state=");
		printflags64(btrfs_balance_state, balance_args.state,
			     "BTRFS_BALANCE_STATE_???");
	}

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
	tprints("{compat_flags=");
	printflags64(btrfs_features_compat, flags->compat_flags,
		     "BTRFS_FEATURE_COMPAT_???");

	tprints(", compat_ro_flags=");
	printflags64(btrfs_features_compat_ro, flags->compat_ro_flags,
		     "BTRFS_FEATURE_COMPAT_RO_???");

	tprints(", incompat_flags=");
	printflags64(btrfs_features_incompat, flags->incompat_flags,
		     "BTRFS_FEATURE_INCOMPAT_???");
	tprints("}");
}

static void
btrfs_print_qgroup_limit(const struct btrfs_qgroup_limit *lim)
{
	tprints("{flags=");
	printflags64(btrfs_qgroup_limit_flags, lim->flags,
		     "BTRFS_QGROUP_LIMIT_???");
	tprintf(", max_rfer=%" PRI__u64 ", max_excl=%" PRI__u64
		", rsv_rfer=%" PRI__u64 ", rsv_excl=%" PRI__u64 "}",
		lim->max_rfer, lim->max_excl,
		lim->rsv_rfer, lim->rsv_excl);
}

static void
btrfs_print_key_type(uint32_t type)
{
	tprintf("%u", type);
	tprints_comment(xlookup(btrfs_key_types, type));
}

static void
btrfs_print_objectid(uint64_t objectid)
{
	tprintf("%" PRIu64, objectid);
	tprints_comment(xlookup(btrfs_tree_objectids, objectid));
}

static void
btrfs_print_data_container_header(const struct btrfs_data_container *container)
{
	tprintf("{bytes_left=%u, bytes_missing=%u"
		", elem_cnt=%u, elem_missed=%u, val=",
		container->bytes_left, container->bytes_missing,
		container->elem_cnt, container->elem_missed);
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
	const uint64_t *const record = elem_buf;

	tprintf("{inum=%" PRIu64 ", offset=%" PRIu64 ", root=%" PRIu64 "}",
		record[0], record[1], record[2]);

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
		tprints("...");
	} else {
		const uint64_t val_addr =
			inodes_addr + offsetof(typeof(container), val);
		uint64_t record[3];
		print_array(tcp, val_addr, container.elem_cnt / 3,
			    record, sizeof(record),
			    umoven_or_printaddr,
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
		tprints("...");
	} else {
		uint64_t val_addr =
			fspath_addr + offsetof(typeof(container), val);
		uint64_t offset;
		print_array(tcp, val_addr, container.elem_cnt,
			    &offset, sizeof(offset),
			    umoven_or_printaddr,
			    print_btrfs_data_container_ino_path, &val_addr);
	}

	btrfs_print_data_container_footer();
}

static bool
print_uint64(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	tprintf("%" PRIu64, * (uint64_t *) elem_buf);

	return true;
}

static void
btrfs_print_qgroup_inherit(struct tcb *const tcp, const kernel_ulong_t qgi_addr)
{
	struct btrfs_qgroup_inherit inherit;

	if (umove_or_printaddr(tcp, qgi_addr, &inherit))
		return;

	tprints("{flags=");
	printflags64(btrfs_qgroup_inherit_flags, inherit.flags,
		     "BTRFS_QGROUP_INHERIT_???");
	tprintf(", num_qgroups=%" PRI__u64 ", num_ref_copies=%" PRI__u64
		", num_excl_copies=%" PRI__u64 ", lim=",
		inherit.num_qgroups, inherit.num_ref_copies,
		inherit.num_excl_copies);

	btrfs_print_qgroup_limit(&inherit.lim);

	tprints(", qgroups=");

	if (abbrev(tcp)) {
		tprints("...");
	} else {
		uint64_t record;
		print_array(tcp, qgi_addr + offsetof(typeof(inherit), qgroups),
			    inherit.num_qgroups, &record, sizeof(record),
			    umoven_or_printaddr, print_uint64, 0);
	}
	tprints("}");
}

static void
print_key_value_internal(struct tcb *tcp, const char *name, uint64_t value)
{
	if (value) {
		tprintf(", %s=%" PRIu64, name, value);
		if (value == UINT64_MAX)
			tprints_comment("UINT64_MAX");
	}
}
#define print_key_value(tcp, key, name)					\
	print_key_value_internal((tcp), #name, (key)->name)

static void
btrfs_print_tree_search(struct tcb *tcp, struct btrfs_ioctl_search_key *key,
			uint64_t buf_addr, uint64_t buf_size, bool print_size)
{
	if (entering(tcp)) {
		tprints("{key={tree_id=");
		btrfs_print_objectid(key->tree_id);

		if (key->min_objectid != BTRFS_FIRST_FREE_OBJECTID ||
		    !abbrev(tcp)) {
			tprints(", min_objectid=");
			btrfs_print_objectid(key->min_objectid);
		}

		if (key->max_objectid != BTRFS_LAST_FREE_OBJECTID ||
		    !abbrev(tcp)) {
			tprints(", max_objectid=");
			btrfs_print_objectid(key->max_objectid);
		}

		print_key_value(tcp, key, min_offset);
		print_key_value(tcp, key, max_offset);
		print_key_value(tcp, key, min_transid);
		print_key_value(tcp, key, max_transid);

		tprints(", min_type=");
		btrfs_print_key_type(key->min_type);
		tprints(", max_type=");
		btrfs_print_key_type(key->max_type);
		tprintf(", nr_items=%u}", key->nr_items);
		if (print_size)
			tprintf(", buf_size=%" PRIu64, buf_size);
		tprints("}");
	} else {
		tprintf("{key={nr_items=%u}", key->nr_items);
		if (print_size)
			tprintf(", buf_size=%" PRIu64, buf_size);
		tprints(", buf=");
		if (abbrev(tcp))
			tprints("...");
		else {
			uint64_t i;
			uint64_t off = 0;
			tprints("[");
			for (i = 0; i < key->nr_items; i++) {
				struct btrfs_ioctl_search_header sh;
				uint64_t addr = buf_addr + off;
				if (i)
					tprints(", ");
				if (i > max_strlen ||
				    umove(tcp, addr, &sh)) {
					tprints("...");
					break;
				}
				tprintf("{transid=%" PRI__u64 ", objectid=",
					sh.transid);
				btrfs_print_objectid(sh.objectid);
				tprintf(", offset=%" PRI__u64 ", type=", sh.offset);
				btrfs_print_key_type(sh.type);
				tprintf(", len=%u}", sh.len);
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
	btrfs_print_objectid(* (uint64_t *) elem_buf);

	return true;
}

static bool
print_btrfs_ioctl_space_info(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	const struct btrfs_ioctl_space_info *info = elem_buf;

	tprints("{flags=");
	printflags64(btrfs_space_info_flags, info->flags,
		     "BTRFS_SPACE_INFO_???");
	tprintf(", total_bytes=%" PRI__u64 ", used_bytes=%" PRI__u64 "}",
		info->total_bytes, info->used_bytes);

	return true;
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
	/* fall through */
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

		tprintf("{start=%" PRIu64 ", len=", (uint64_t)args.start);

		tprintf("%" PRIu64, (uint64_t) args.len);
		if (args.len == UINT64_MAX)
			tprints_comment("UINT64_MAX");

		tprints(", flags=");
		printflags64(btrfs_defrag_flags, args.flags,
			     "BTRFS_DEFRAG_RANGE_???");
		tprintf(", extent_thresh=%u, compress_type=",
			args.extent_thresh);
		printxval(btrfs_compress_types, args.compress_type,
			  "BTRFS_COMPRESS_???");
		tprints("}");
		break;
	}

	case BTRFS_IOC_DEV_INFO: { /* RW */
		struct btrfs_ioctl_dev_info_args args;
		char uuid[UUID_STRING_SIZE+1];
		int valid;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;
		tprints("{");

		valid = btrfs_unparse_uuid(args.uuid, uuid);
		if (entering(tcp)) {
			tprintf("devid=%" PRI__u64, args.devid);
			if (valid)
				tprintf(", uuid=%s", uuid);
			tprints("}");
			return 0;
		}
		if (valid)
			tprintf("uuid=%s, ", uuid);
		tprintf("bytes_used=%" PRI__u64
			", total_bytes=%" PRI__u64 ", path=",
			args.bytes_used, args.total_bytes);
		print_quoted_string((const char *)args.path, sizeof(args.path),
				    QUOTE_0_TERMINATED);
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
			tprints("{cmd=");
			printxval64(btrfs_dev_replace_cmds, args.cmd,
				    "BTRFS_IOCTL_DEV_REPLACE_CMD_???");
			if (args.cmd == BTRFS_IOCTL_DEV_REPLACE_CMD_START) {
				const char *str;
				tprintf(", start={srcdevid=%" PRIu64
				   ", cont_reading_from_srcdev_mode=%" PRIu64
				   ", srcdev_name=",
				   (uint64_t) args.start.srcdevid,
				   (uint64_t) args.start.cont_reading_from_srcdev_mode);

				str = (const char*) args.start.srcdev_name;
				print_quoted_string(str,
						sizeof(args.start.srcdev_name),
						QUOTE_0_TERMINATED);
				tprints(", tgtdev_name=");
				str = (const char*) args.start.tgtdev_name;
				print_quoted_string(str,
						sizeof(args.start.tgtdev_name),
						QUOTE_0_TERMINATED);
				tprints("}");

			}
			tprints("}");
			return 0;
		}

		tprints("{result=");
		printxval64(btrfs_dev_replace_results, args.result,
			    "BTRFS_IOCTL_DEV_REPLACE_RESULT_???");
		if (args.cmd == BTRFS_IOCTL_DEV_REPLACE_CMD_STATUS) {
			tprints(", ");
			printxval64(btrfs_dev_replace_state,
				   args.status.replace_state,
				   "BTRFS_IOCTL_DEV_REPLACE_STATE_???");
			tprintf(", progress_1000=%" PRIu64,
				(uint64_t) args.status.progress_1000);

			if (args.status.progress_1000 <= 1000)
				tprintf_comment("%u.%u%%",
					(unsigned) args.status.progress_1000 / 10,
					(unsigned) args.status.progress_1000 % 10);

			tprintf(", time_started=%" PRIu64,
				(uint64_t) args.status.time_started);
			tprints_comment(sprinttime(args.status.time_started));

			tprintf(", time_stopped=%" PRIu64,
				(uint64_t) args.status.time_stopped);
			tprints_comment(sprinttime(args.status.time_stopped));

			tprintf(", num_write_errors=%" PRIu64
				", num_uncorrectable_read_errors=%" PRIu64,
				(uint64_t) args.status.num_write_errors,
				(uint64_t) args.status.num_uncorrectable_read_errors);
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
		char uuid[UUID_STRING_SIZE+1];
		uint32_t nodesize, sectorsize, clone_alignment;
#ifndef HAVE_STRUCT_BTRFS_IOCTL_FS_INFO_ARGS_NODESIZE
		__u32 *reserved32;
#endif

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

#ifdef HAVE_STRUCT_BTRFS_IOCTL_FS_INFO_ARGS_NODESIZE
		nodesize = args.nodesize,
		sectorsize = args.sectorsize,
		clone_alignment = args.clone_alignment;
#else
		reserved32 = (__u32 *) (void *) args.reserved;
		nodesize = reserved32[0];
		sectorsize = reserved32[1];
		clone_alignment = reserved32[2];
#endif
		btrfs_unparse_uuid(args.fsid, uuid);

		tprints("{");
		tprintf("max_id=%" PRI__u64 ", num_devices=%" PRI__u64
			", fsid=%s, nodesize=%u, sectorsize=%u"
			", clone_alignment=%u",
			args.max_id, args.num_devices, uuid,
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

		if (entering(tcp))
			tprintf("devid=%" PRI__u64 ", ", args.devid);

		tprintf("nr_items=%" PRI__u64 ", flags=", args.nr_items);
		printflags64(btrfs_dev_stats_flags, args.flags,
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
			tprintf("%" PRI__u64, args.values[i]);
			tprints_comment(xlookup(btrfs_dev_stats_values, i));
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

			tprints("{treeid=");
			btrfs_print_objectid(args.treeid);
			tprints(", objectid=");
			btrfs_print_objectid(args.objectid);
			tprints("}");
			return 0;
		}

		tprints("{");
		if (get_tcb_priv_ulong(tcp)) {
			tprints("treeid=");
			btrfs_print_objectid(args.treeid);
			tprints(", ");
		}

		tprints("name=");
		print_quoted_string(args.name, sizeof(args.name),
				    QUOTE_0_TERMINATED);
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

		tprints("{");

		if (entering(tcp)) {
			tprintf("inum=%" PRI__u64 ", size=%" PRI__u64,
				args.inum, args.size);
			tprintf(", fspath=0x%" PRI__x64 "}", args.fspath);
			return 0;
		}

		tprints("fspath=");
		btrfs_print_ino_path_container(tcp, args.fspath);

		tprints("}");
		break;
	}

	case BTRFS_IOC_LOGICAL_INO: { /* RW */
		struct btrfs_ioctl_logical_ino_args args;

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
			tprintf("logical=%" PRI__u64 ", size=%" PRI__u64,
				args.logical, args.size);
			tprintf(", inodes=0x%" PRI__x64 "}", args.inodes);
			return 0;
		}

		tprints("inodes=");
		btrfs_print_logical_ino_container(tcp, args.inodes);

		tprints("}");
		break;
	}

	case BTRFS_IOC_QGROUP_ASSIGN: { /* W */
		struct btrfs_ioctl_qgroup_assign_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprintf("{assign=%" PRI__u64 ", src=%" PRI__u64
			", dst=%" PRI__u64 "}",
			args.assign, args.src, args.dst);
		break;
	}

	case BTRFS_IOC_QGROUP_CREATE: { /* W */
		struct btrfs_ioctl_qgroup_create_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprintf("{create=%" PRI__u64 ", qgroupid=%" PRI__u64 "}",
			args.create, args.qgroupid);
		break;
	}

	case BTRFS_IOC_QGROUP_LIMIT: { /* R */
		struct btrfs_ioctl_qgroup_limit_args args;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprintf("{qgroupid=%" PRI__u64 ", lim=", args.qgroupid);
		btrfs_print_qgroup_limit(&args.lim);
		tprints("}");
		break;
	}

	case BTRFS_IOC_QUOTA_CTL: { /* W */
		struct btrfs_ioctl_quota_ctl_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		printxval64(btrfs_qgroup_ctl_cmds, args.cmd,
			    "BTRFS_QUOTA_CTL_???");
		tprints("}");

		break;
	}

	case BTRFS_IOC_QUOTA_RESCAN: { /* W */
		struct btrfs_ioctl_quota_rescan_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprintf("{flags=%" PRIu64 "}", (uint64_t) args.flags);
		break;
	}

	case BTRFS_IOC_QUOTA_RESCAN_STATUS: { /* R */
		struct btrfs_ioctl_quota_rescan_args args;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprintf("{flags=%" PRIu64 ", progress=", (uint64_t) args.flags);
		btrfs_print_objectid(args.progress);
		tprints("}");
		break;
	}

	case BTRFS_IOC_SET_RECEIVED_SUBVOL: { /* RW */
		struct_btrfs_ioctl_received_subvol_args args;
		char uuid[UUID_STRING_SIZE+1];

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			btrfs_unparse_uuid((unsigned char *)args.uuid, uuid);
			tprintf("{uuid=%s, stransid=%" PRIu64
				", stime=%" PRIu64 ".%u, flags=%" PRIu64
				"}", uuid, (uint64_t) args.stransid,
				(uint64_t) args.stime.sec, args.stime.nsec,
				(uint64_t) args.flags);
			return 0;
		}
		tprintf("{rtransid=%" PRIu64 ", rtime=%" PRIu64 ".%u}",
			(uint64_t) args.rtransid, (uint64_t) args.rtime.sec,
			args.rtime.nsec);
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
			tprintf("{devid=%" PRI__u64, args.devid);
			if (code == BTRFS_IOC_SCRUB) {
				tprintf(", start=%" PRI__u64 ", end=",
					args.start);
				tprintf("%" PRI__u64, args.end);
				if (args.end == UINT64_MAX)
					tprints_comment("UINT64_MAX");
				tprints(", flags=");
				printflags64(btrfs_scrub_flags, args.flags,
					     "BTRFS_SCRUB_???");
			}
			tprints("}");
			return 0;
		}
		tprintf("{data_extents_scrubbed=%" PRI__u64
			", tree_extents_scrubbed=%" PRI__u64
			", data_bytes_scrubbed=%" PRI__u64
			", tree_bytes_scrubbed=%" PRI__u64
			", read_errors=%" PRI__u64
			", csum_errors=%" PRI__u64
			", verify_errors=%" PRI__u64
			", no_csum=%" PRI__u64
			", csum_discards=%" PRI__u64
			", super_errors=%" PRI__u64
			", malloc_errors=%" PRI__u64
			", uncorrectable_errors=%" PRI__u64
			", corrected_errors=%" PRI__u64
			", last_physical=%" PRI__u64
			", unverified_errors=%" PRI__u64 "}",
			args.progress.data_extents_scrubbed,
			args.progress.tree_extents_scrubbed,
			args.progress.data_bytes_scrubbed,
			args.progress.tree_bytes_scrubbed,
			args.progress.read_errors,
			args.progress.csum_errors,
			args.progress.verify_errors,
			args.progress.no_csum,
			args.progress.csum_discards,
			args.progress.super_errors,
			args.progress.malloc_errors,
			args.progress.uncorrectable_errors,
			args.progress.corrected_errors,
			args.progress.last_physical,
			args.progress.unverified_errors);
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
				tcp->u_error = 0;
				if (!umove_or_printaddr(tcp, arg, &args))
					tprintf("{buf_size=%" PRIu64 "}",
						(uint64_t)args.buf_size);
				tcp->u_error = EOVERFLOW;
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

		tprints("{send_fd=");
		printfd(tcp, args.send_fd);
		tprintf(", clone_sources_count=%" PRIu64 ", clone_sources=",
			(uint64_t) args.clone_sources_count);

		if (abbrev(tcp))
			tprints("...");
		else {
			uint64_t record;
			print_array(tcp, ptr_to_kulong(args.clone_sources),
				    args.clone_sources_count,
				    &record, sizeof(record),
				    umoven_or_printaddr,
				    print_objectid_callback, 0);
		}
		tprints(", parent_root=");
		btrfs_print_objectid(args.parent_root);
		tprints(", flags=");
		printflags64(btrfs_send_flags, args.flags,
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

		tprints("{");
		if (entering(tcp)) {
			tprintf("space_slots=%" PRI__u64 "}", args.space_slots);
			return 0;
		}

		tprintf("total_spaces=%" PRI__u64, args.total_spaces);

		if (args.space_slots == 0 && args.total_spaces) {
			tprints("}");
			break;
		}

		tprints(", spaces=");

		if (abbrev(tcp))
			tprints("...");
		else {
			struct btrfs_ioctl_space_info info;
			print_array(tcp, arg + offsetof(typeof(args), spaces),
				    args.total_spaces,
				    &info, sizeof(info), umoven_or_printaddr,
				    print_btrfs_ioctl_space_info, 0);
		}
		tprints("}");
		break;
	}

	case BTRFS_IOC_SNAP_CREATE:
	case BTRFS_IOC_RESIZE:
	case BTRFS_IOC_SCAN_DEV:
	case BTRFS_IOC_ADD_DEV:
	case BTRFS_IOC_RM_DEV:
	case BTRFS_IOC_SUBVOL_CREATE:
	case BTRFS_IOC_SNAP_DESTROY:
	case BTRFS_IOC_DEVICES_READY: { /* W */
		struct btrfs_ioctl_vol_args args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		tprints("{fd=");
		printfd(tcp, args.fd);
		tprints(", name=");
		print_quoted_string(args.name, sizeof(args.name),
				    QUOTE_0_TERMINATED);
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
			tprints("{fd=");
			printfd(tcp, args.fd);
			tprints(", flags=");
			printflags64(btrfs_snap_flags_v2, args.flags,
				     "BTRFS_SUBVOL_???");
			if (args.flags & BTRFS_SUBVOL_QGROUP_INHERIT) {
				tprintf(", size=%" PRIu64 ", qgroup_inherit=",
					(uint64_t) args.size);

				btrfs_print_qgroup_inherit(tcp,
					ptr_to_kulong(args.qgroup_inherit));
			}
			tprints(", name=");
			print_quoted_string(args.name, sizeof(args.name),
					    QUOTE_0_TERMINATED);
			tprints("}");
			return 0;
		}
		tprintf("{transid=%" PRIu64 "}", (uint64_t) args.transid);
		break;
	}

	case BTRFS_IOC_GET_FSLABEL: /* R */
		if (entering(tcp))
			return 0;
		/* fall through */
	case BTRFS_IOC_SET_FSLABEL: { /* W */
		char label[BTRFS_LABEL_SIZE];

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &label))
			break;
		print_quoted_string(label, sizeof(label), QUOTE_0_TERMINATED);
		break;
	}

	case BTRFS_IOC_CLONE:			/* FICLONE */
	case BTRFS_IOC_CLONE_RANGE:		/* FICLONERANGE */
#ifdef BTRFS_IOC_FILE_EXTENT_SAME
	case BTRFS_IOC_FILE_EXTENT_SAME:	/* FIDEDUPERANGE */
#endif
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
	return RVAL_DECODED | 1;
}
#endif /* HAVE_LINUX_BTRFS_H */
