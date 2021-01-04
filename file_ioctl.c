/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include <linux/fs.h>
#include "types/fiemap.h"
#include "xlat/fiemap_flags.h"
#include "xlat/fiemap_extent_flags.h"
#define XLAT_MACROS_ONLY
# include "xlat/fs_f_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static bool
print_fiemap_extent(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct_fiemap_extent *fe = elem_buf;

	PRINT_FIELD_U("{", *fe, fe_logical);
	PRINT_FIELD_U(", ", *fe, fe_physical);
	PRINT_FIELD_U(", ", *fe, fe_length);
	PRINT_FIELD_FLAGS(", ", *fe, fe_flags, fiemap_extent_flags,
			  "FIEMAP_EXTENT_???");
	tprints("}");

	return true;
}

int
file_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case FS_IOC_FIEMAP: {
		struct_fiemap args;

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
			struct_fiemap_extent fe;
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

	default:
		return RVAL_DECODED;
	};

	return RVAL_IOCTL_DECODED;
}
