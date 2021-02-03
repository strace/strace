/*
 * Type definitions related to linux/fs.h 0x94 ioctl commands.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_FS_0X94_H
# define STRACE_TYPES_FS_0X94_H

# include <linux/fs.h>

typedef struct {
	int64_t src_fd;
	uint64_t src_offset;
	uint64_t src_length;
	uint64_t dest_offset;
} struct_file_clone_range;

typedef struct {
	int64_t dest_fd;
	uint64_t dest_offset;
	uint64_t bytes_deduped;
	int32_t status;
	uint32_t reserved;
} struct_file_dedupe_range_info;

typedef struct {
	uint64_t src_offset;
	uint64_t src_length;
	uint16_t dest_count;
	uint16_t reserved1;
	uint32_t reserved2;
	struct_file_dedupe_range_info info[0];
} struct_file_dedupe_range;

typedef char fs_0x94_label_t[256];

#endif /* STRACE_TYPES_FS_0X94_H */
