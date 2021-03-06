/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fs.h>

static void
decode_file_clone_range(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct file_clone_range range;

	if (!umove_or_printaddr(tcp, arg, &range)) {
		tprint_struct_begin();
		PRINT_FIELD_FD(range, src_fd, tcp);
		tprint_struct_next();
		PRINT_FIELD_U(range, src_offset);
		tprint_struct_next();
		PRINT_FIELD_U(range, src_length);
		tprint_struct_next();
		PRINT_FIELD_U(range, dest_offset);
		tprint_struct_end();
	}
}

static bool
print_file_dedupe_range_info(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	const struct file_dedupe_range_info *info = elem_buf;
	unsigned int *count = data;

	if (count) {
		if (*count == 0) {
			tprint_more_data_follows();
			return false;
		}
		--*count;
	}

	if (entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_FD(*info, dest_fd, tcp);
		tprint_struct_next();
		PRINT_FIELD_U(*info, dest_offset);
	} else {
		tprint_struct_begin();
		PRINT_FIELD_U(*info, bytes_deduped);
		tprint_struct_next();
		PRINT_FIELD_D(*info, status);
	}
	tprint_struct_end();

	return true;
}

static int
decode_file_dedupe_range(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct file_dedupe_range range;
	struct file_dedupe_range_info info;
	unsigned int *limit = NULL;
	unsigned int count = 2;
	bool rc;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &range))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();

	if (entering(tcp)) {
		PRINT_FIELD_U(range, src_offset);
		tprint_struct_next();
		PRINT_FIELD_U(range, src_length);
		tprint_struct_next();
		PRINT_FIELD_U(range, dest_count);
		tprint_struct_next();
	}

	tprints_field_name("info");

	/* Limit how many elements we print in abbrev mode. */
	if (abbrev(tcp) && range.dest_count > count)
		limit = &count;

	rc = print_array(tcp, arg + offsetof(typeof(range), info),
			 range.dest_count, &info, sizeof(info),
			 tfetch_mem,
			 print_file_dedupe_range_info, limit);

	tprint_struct_end();

	if (!rc || exiting(tcp))
		return RVAL_IOCTL_DECODED;

	return 0;
}

static void
decode_fslabel(struct tcb *const tcp, const kernel_ulong_t arg)
{
	char label[FSLABEL_MAX];

	if (!umove_or_printaddr(tcp, arg, &label))
		print_quoted_cstring(label, sizeof(label));
}

int
fs_0x94_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	switch (code) {
	case FICLONE:	/* W */
		tprint_arg_next();
		PRINT_VAL_D((int) arg);
		break;

	case FICLONERANGE:	/* W */
		tprint_arg_next();
		decode_file_clone_range(tcp, arg);
		break;

	case FIDEDUPERANGE:	/* WR */
		return decode_file_dedupe_range(tcp, arg);

	case FS_IOC_GETFSLABEL:	/* R */
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;

	case FS_IOC_SETFSLABEL:	/* W */
		tprint_arg_next();
		decode_fslabel(tcp, arg);
		break;

	default:
		return btrfs_ioctl(tcp, code, arg);
	};

	return RVAL_IOCTL_DECODED;
}
