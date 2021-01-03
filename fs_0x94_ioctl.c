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

int
fs_0x94_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	switch (code) {
	case FICLONE:	/* W */
		tprintf(", %d", (int) arg);
		break;

	case FICLONERANGE: { /* W */
		struct_file_clone_range args;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &args))
			break;

		PRINT_FIELD_FD("{", args, src_fd, tcp);
		PRINT_FIELD_U(", ", args, src_offset);
		PRINT_FIELD_U(", ", args, src_length);
		PRINT_FIELD_U(", ", args, dest_offset);
		tprints("}");
		break;
	}

	case FIDEDUPERANGE: { /* RW */
		struct_file_dedupe_range args;
		struct_file_dedupe_range_info info;
		unsigned int *limit = NULL;
		unsigned int count = 2;
		bool rc;

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
			PRINT_FIELD_U("", args, src_offset);
			PRINT_FIELD_U(", ", args, src_length);
			PRINT_FIELD_U(", ", args, dest_count);
			tprints(", ");
		}

		tprints("info=");

		/* Limit how many elements we print in abbrev mode. */
		if (abbrev(tcp) && args.dest_count > count)
			limit = &count;

		rc = print_array(tcp, arg + offsetof(typeof(args), info),
				 args.dest_count, &info, sizeof(info),
				 tfetch_mem,
				 print_file_dedupe_range_info, limit);

		tprints("}");
		if (!rc || exiting(tcp))
			break;

		return 0;
	}

	default:
		return RVAL_DECODED;
	};

	return RVAL_IOCTL_DECODED;
}
