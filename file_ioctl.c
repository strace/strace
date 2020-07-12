/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include <linux/ioctl.h>
#include <linux/fs.h>

#ifdef HAVE_LINUX_FIEMAP_H
# include <linux/types.h>
# include <linux/fiemap.h>
# include "xlat/fiemap_flags.h"
# include "xlat/fiemap_extent_flags.h"
#endif

#ifndef FICLONE
# define FICLONE	_IOW(0x94, 9, int)
#endif

#ifndef FICLONERANGE
# define FICLONERANGE	_IOW(0x94, 13, struct file_clone_range)
struct file_clone_range {
	int64_t src_fd;
	uint64_t src_offset;
	uint64_t src_length;
	uint64_t dest_offset;
};
#endif

#ifndef FIDEDUPERANGE
# define FIDEDUPERANGE	_IOWR(0x94, 54, struct file_dedupe_range)
struct file_dedupe_range_info {
	int64_t dest_fd;	/* in - destination file */
	uint64_t dest_offset;	/* in - start of extent in destination */
	uint64_t bytes_deduped;	/* out - total # of bytes we were able
				 * to dedupe from this file. */
	/* status of this dedupe operation:
	 * < 0 for error
	 * == FILE_DEDUPE_RANGE_SAME if dedupe succeeds
	 * == FILE_DEDUPE_RANGE_DIFFERS if data differs
	 */
	int32_t status;		/* out - see above description */
	uint32_t reserved;	/* must be zero */
};

struct file_dedupe_range {
	uint64_t src_offset;	/* in - start of extent in source */
	uint64_t src_length;	/* in - length of extent */
	uint16_t dest_count;	/* in - total elements in info array */
	uint16_t reserved1;	/* must be zero */
	uint32_t reserved2;	/* must be zero */
	struct file_dedupe_range_info info[0];
};
#endif

static bool
print_file_dedupe_range_info(struct tcb *tcp, void *elem_buf,
			     size_t elem_size, void *data)
{
	const struct file_dedupe_range_info *info = elem_buf;
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

#ifdef HAVE_LINUX_FIEMAP_H
static bool
print_fiemap_extent(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct fiemap_extent *fe = elem_buf;

	PRINT_FIELD_U("{", *fe, fe_logical);
	PRINT_FIELD_U(", ", *fe, fe_physical);
	PRINT_FIELD_U(", ", *fe, fe_length);
	PRINT_FIELD_FLAGS(", ", *fe, fe_flags, fiemap_extent_flags,
			  "FIEMAP_EXTENT_???");
	tprints("}");

	return true;
}
#endif /* HAVE_LINUX_FIEMAP_H */

int
file_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case FICLONE:	/* W */
		tprintf(", %d", (int) arg);
		break;

	case FICLONERANGE: { /* W */
		struct file_clone_range args;

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
		struct file_dedupe_range args;
		struct file_dedupe_range_info info;
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

#ifdef HAVE_LINUX_FIEMAP_H
	case FS_IOC_FIEMAP: {
		struct fiemap args;

		if (entering(tcp))
			tprints(", ");
		else if (syserror(tcp))
			break;
		else
			tprints(" => ");

		if (umove_or_printaddr(tcp, arg, &args))
			break;

		if (entering(tcp)) {
			PRINT_FIELD_U("{", args, fm_start);
			PRINT_FIELD_U(", ", args, fm_length);
			PRINT_FIELD_FLAGS(", ", args, fm_flags, fiemap_flags,
					  "FIEMAP_FLAG_???");
			PRINT_FIELD_U(", ", args, fm_extent_count);
			tprints("}");
			return 0;
		}

		PRINT_FIELD_FLAGS("{", args, fm_flags, fiemap_flags,
				  "FIEMAP_FLAG_???");
		PRINT_FIELD_U(", ", args, fm_mapped_extents);
		if (abbrev(tcp)) {
			tprints(", ...");
		} else {
			struct fiemap_extent fe;
			tprints(", fm_extents=");
			print_array(tcp,
				    arg + offsetof(typeof(args), fm_extents),
				    args.fm_mapped_extents, &fe, sizeof(fe),
				    tfetch_mem,
				    print_fiemap_extent, 0);
		}
		tprints("}");

		break;
	}
#endif /* HAVE_LINUX_FIEMAP_H */

	default:
		return RVAL_DECODED;
	};

	return RVAL_IOCTL_DECODED;
}
