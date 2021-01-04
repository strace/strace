/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_FIEMAP_H
# define STRACE_TYPES_FIEMAP_H

# ifdef HAVE_LINUX_FIEMAP_H
#  include <linux/types.h>
#  include <linux/fiemap.h>
# endif

typedef struct {
	uint64_t fe_logical;
	uint64_t fe_physical;
	uint64_t fe_length;
	uint64_t fe_reserved64[2];
	uint32_t fe_flags;
	uint32_t fe_reserved[3];
} struct_fiemap_extent;

typedef struct {
	uint64_t fm_start;
	uint64_t fm_length;
	uint32_t fm_flags;
	uint32_t fm_mapped_extents;
	uint32_t fm_extent_count;
	uint32_t fm_reserved;
	struct fiemap_extent fm_extents[0];
} struct_fiemap;

#endif /* STRACE_TYPES_FIEMAP_H */
