/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "types/fs_0x94.h"
#define XLAT_MACROS_ONLY
# include "xlat/fs_0x94_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static void
decode_file_clone_range(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_file_clone_range range;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &range)) {
		PRINT_FIELD_FD("{", range, src_fd, tcp);
		PRINT_FIELD_U(", ", range, src_offset);
		PRINT_FIELD_U(", ", range, src_length);
		PRINT_FIELD_U(", ", range, dest_offset);
		tprints("}");
	}
}

static bool
print_file_dedupe_range_info(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	const struct_file_dedupe_range_info *info = elem_buf;
	unsigned int *count = data;

	if (count) {
		if (*count == 0) {
			tprints("...");
			return false;
		}
		--*count;
	}

	if (entering(tcp)) {
		PRINT_FIELD_FD("{", *info, dest_fd, tcp);
		PRINT_FIELD_U(", ", *info, dest_offset);
	} else {
		PRINT_FIELD_U("{", *info, bytes_deduped);
		PRINT_FIELD_D(", ", *info, status);
	}
	tprints("}");

	return true;
}

static int
decode_file_dedupe_range(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_file_dedupe_range range;
	struct_file_dedupe_range_info info;
	unsigned int *limit = NULL;
	unsigned int count = 2;
	bool rc;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &range))
		return RVAL_IOCTL_DECODED;

	tprints("{");

	if (entering(tcp)) {
		PRINT_FIELD_U("", range, src_offset);
		PRINT_FIELD_U(", ", range, src_length);
		PRINT_FIELD_U(", ", range, dest_count);
		tprints(", ");
	}

	tprints("info=");

	/* Limit how many elements we print in abbrev mode. */
	if (abbrev(tcp) && range.dest_count > count)
		limit = &count;

	rc = print_array(tcp, arg + offsetof(typeof(range), info),
			 range.dest_count, &info, sizeof(info),
			 tfetch_mem,
			 print_file_dedupe_range_info, limit);

	tprints("}");

	if (!rc || exiting(tcp))
		return RVAL_IOCTL_DECODED;

	return 0;
}

int
fs_0x94_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	switch (code) {
	case FICLONE:	/* W */
		tprintf(", %d", (int) arg);
		break;

	case FICLONERANGE:	/* W */
		decode_file_clone_range(tcp, arg);
		break;

	case FIDEDUPERANGE:	/* WR */
		return decode_file_dedupe_range(tcp, arg);

	default:
#ifdef HAVE_LINUX_BTRFS_H
		return btrfs_ioctl(tcp, code, arg);
#else
		return RVAL_DECODED;
#endif
	};

	return RVAL_IOCTL_DECODED;
}
