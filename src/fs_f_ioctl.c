/*
 * Copyright (c) 2016 Jeff Mahoney <jeffm@suse.com>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fs.h>
#include <linux/fiemap.h>
#include "xlat/fiemap_flags.h"
#include "xlat/fiemap_extent_flags.h"
#include "xlat/fs_ioc_flags.h"

static bool
print_fiemap_extent(struct tcb *tcp, void *elem_buf, size_t elem_size, void *data)
{
	const struct fiemap_extent *fe = elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_U(*fe, fe_logical);
	tprint_struct_next();
	PRINT_FIELD_U(*fe, fe_physical);
	tprint_struct_next();
	PRINT_FIELD_U(*fe, fe_length);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*fe, fe_flags, fiemap_extent_flags,
			  "FIEMAP_EXTENT_???");
	tprint_struct_end();

	return true;
}

static int
decode_fiemap(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct fiemap args;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &args))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(args, fm_start);
		tprint_struct_next();
		PRINT_FIELD_U(args, fm_length);
		tprint_struct_next();
		PRINT_FIELD_FLAGS(args, fm_flags, fiemap_flags,
				  "FIEMAP_FLAG_???");
		tprint_struct_next();
		PRINT_FIELD_U(args, fm_extent_count);
		tprint_struct_end();
		return 0;
	}

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(args, fm_flags, fiemap_flags, "FIEMAP_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_U(args, fm_mapped_extents);
	if (abbrev(tcp)) {
		tprint_struct_next();
		tprint_more_data_follows();
	} else {
		struct fiemap_extent fe;
		tprint_struct_next();
		tprints_field_name("fm_extents");
		print_array(tcp, arg + offsetof(typeof(args), fm_extents),
			    args.fm_mapped_extents, &fe, sizeof(fe),
			    tfetch_mem, print_fiemap_extent, 0);
	}
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static void
decode_fs_ioc_flags(struct tcb *const tcp, const kernel_ulong_t arg)
{
	unsigned int flags;

	if (!umove_or_printaddr(tcp, arg, &flags)) {
		tprint_indirect_begin();
		printflags(fs_ioc_flags, flags, "FS_???_FL");
		tprint_indirect_end();
	}
}

int
fs_f_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case FS_IOC_FIEMAP:
		return decode_fiemap(tcp, arg);

	case FS_IOC_GETFLAGS:
#if SIZEOF_LONG > 4
	case FS_IOC32_GETFLAGS:
#endif
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;

	case FS_IOC_SETFLAGS:
#if SIZEOF_LONG > 4
	case FS_IOC32_SETFLAGS:
#endif
		tprint_arg_next();
		decode_fs_ioc_flags(tcp, arg);
		break;

	default:
		return RVAL_DECODED;
	};

	return RVAL_IOCTL_DECODED;
}
