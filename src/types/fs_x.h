/*
 * Type definitions related to linux/fs.h 'X' ioctl commands.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_FS_X_H
# define STRACE_TYPES_FS_X_H

# include <linux/fs.h>

typedef struct {
	uint64_t start;
	uint64_t len;
	uint64_t minlen;
} struct_fstrim_range;

typedef struct {
	uint32_t fsx_xflags;
	uint32_t fsx_extsize;
	uint32_t fsx_nextents;
	uint32_t fsx_projid;
	uint32_t fsx_cowextsize;
	unsigned char fsx_pad[8];
} struct_fsxattr;

#endif /* STRACE_TYPES_FS_X_H */
