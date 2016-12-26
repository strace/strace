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
#include <linux/ioctl.h>
#include <linux/fs.h>

#ifdef HAVE_LINUX_FIEMAP_H
# include <linux/fiemap.h>
# include "xlat/fiemap_flags.h"
# include "xlat/fiemap_extent_flags.h"
#endif

#ifndef FICLONE
# define FICLONE         _IOW(0x94, 9, int)
#endif

#ifndef FICLONERANGE
# define FICLONERANGE    _IOW(0x94, 13, struct file_clone_range)
struct file_clone_range {
	int64_t src_fd;
	uint64_t src_offset;
	uint64_t src_length;
	uint64_t dest_offset;
};
#endif

#ifndef FIDEDUPERANGE
# define FIDEDUPERANGE   _IOWR(0x94, 54, struct file_dedupe_range)
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
		tprints("{dest_fd=");
		printfd(tcp, info->dest_fd);
		tprintf(", dest_offset=%" PRIu64 "}",
			(uint64_t) info->dest_offset);
	} else {
		tprintf("{bytes_deduped=%" PRIu64 ", status=%d}",
			(uint64_t) info->bytes_deduped, info->status);
	}

	return true;
}

#ifdef HAVE_LINUX_FIEMAP_H
static bool
print_fiemap_extent(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct fiemap_extent *fe = elem_buf;

	tprintf("{fe_logical=%" PRI__u64
		", fe_physical=%" PRI__u64
		", fe_length=%" PRI__u64 ", ",
		fe->fe_logical, fe->fe_physical, fe->fe_length);

	printflags64(fiemap_extent_flags, fe->fe_flags,
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

		tprints("{src_fd=");
		printfd(tcp, args.src_fd);
		tprintf(", src_offset=%" PRIu64
			", src_length=%" PRIu64
			", dest_offset=%" PRIu64 "}",
			(uint64_t) args.src_offset,
			(uint64_t) args.src_length,
			(uint64_t) args.dest_offset);
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
			tprintf("src_offset=%" PRIu64
				", src_length=%" PRIu64
				", dest_count=%hu, ",
				(uint64_t) args.src_offset,
				(uint64_t) args.src_length,
				(uint16_t) args.dest_count);
		}

		tprints("info=");

		/* Limit how many elements we print in abbrev mode. */
		if (abbrev(tcp) && args.dest_count > count)
			limit = &count;

		rc = print_array(tcp, arg + offsetof(typeof(args), info),
				 args.dest_count, &info, sizeof(info),
				 umoven_or_printaddr,
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
			tprintf("{fm_start=%" PRI__u64 ", "
				"fm_length=%" PRI__u64 ", "
				"fm_flags=",
				args.fm_start, args.fm_length);
			printflags64(fiemap_flags, args.fm_flags,
				     "FIEMAP_FLAG_???");
			tprintf(", fm_extent_count=%u}", args.fm_extent_count);
			return 0;
		}

		tprints("{fm_flags=");
		printflags64(fiemap_flags, args.fm_flags,
			     "FIEMAP_FLAG_???");
		tprintf(", fm_mapped_extents=%u",
			args.fm_mapped_extents);
		tprints(", fm_extents=");
		if (abbrev(tcp)) {
			tprints("...");
		} else {
			struct fiemap_extent fe;
			print_array(tcp,
				    arg + offsetof(typeof(args), fm_extents),
				    args.fm_mapped_extents, &fe, sizeof(fe),
				    umoven_or_printaddr,
				    print_fiemap_extent, 0);
		}
		tprints("}");

		break;
	}
#endif /* HAVE_LINUX_FIEMAP_H */

	default:
		return RVAL_DECODED;
	};

	return RVAL_DECODED | 1;
}
