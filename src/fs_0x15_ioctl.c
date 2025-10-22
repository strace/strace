/*
 * Decoders of linux/fs.h 0x15 ioctl commands.
 *
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/fs.h>

static void
decode_fsuuid2(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct fsuuid2 uuid;

	if (!umove_or_printaddr(tcp, arg, &uuid)) {
		tprint_struct_begin();
		PRINT_FIELD_U(uuid, len);
		tprint_struct_next();
		PRINT_FIELD_STRING(uuid, uuid,
				   MIN(uuid.len, sizeof(uuid.uuid)),
				   QUOTE_FORCE_HEX);
		tprint_struct_end();
	}
}

static void
decode_fs_sysfs_path(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct fs_sysfs_path path;

	if (!umove_or_printaddr(tcp, arg, &path)) {
		tprint_struct_begin();
		PRINT_FIELD_U(path, len);
		tprint_struct_next();
		PRINT_FIELD_CSTRING_SZ(path, name,
				       MIN((unsigned) path.len + 1, sizeof(path.name)));
		tprint_struct_end();
	}
}

int
fs_0x15_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	switch (code) {
	case FS_IOC_GETFSUUID:
		if (entering(tcp))
			return 0;
		tprints_arg_next_name("arg");
		decode_fsuuid2(tcp, arg);
		break;

	case FS_IOC_GETFSSYSFSPATH:
		if (entering(tcp))
			return 0;
		tprints_arg_next_name("arg");
		decode_fs_sysfs_path(tcp, arg);
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
