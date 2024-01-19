/*
 * Copyright (c) 2017-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_STATX_H
# define STRACE_STATX_H

# include <stdint.h>

typedef struct {
	int64_t tv_sec;
	int32_t tv_nsec;
	int32_t reserved;
} struct_statx_timestamp;

typedef struct {
	uint32_t stx_mask; /* What results were written [uncond] */
	uint32_t stx_blksize; /* Preferred general I/O size [uncond] */
	uint64_t stx_attributes; /* Flags conveying information about the file
				    [uncond] */

	uint32_t stx_nlink; /* Number of hard links */
	uint32_t stx_uid; /* User ID of owner */
	uint32_t stx_gid; /* Group ID of owner */
	uint16_t stx_mode; /* File mode */
	uint16_t reserved0[1];

	uint64_t stx_ino; /* Inode number */
	uint64_t stx_size; /* File size */
	uint64_t stx_blocks; /* Number of 512-byte blocks allocated */
	uint64_t stx_attributes_mask; /* Mask to show what's supported in
					 stx_attributes */

	struct_statx_timestamp stx_atime; /* Last access time */
	struct_statx_timestamp stx_btime; /* File creation time */
	struct_statx_timestamp stx_ctime; /* Last attribute change time */
	struct_statx_timestamp stx_mtime; /* Last data modification time */

	uint32_t stx_rdev_major; /* Device ID of special file [if bdev/cdev] */
	uint32_t stx_rdev_minor;
	uint32_t stx_dev_major; /* ID of device containing file [uncond] */
	uint32_t stx_dev_minor;

	uint64_t stx_mnt_id;
	uint32_t stx_dio_mem_align; /* Memory buffer alignment for direct I/O */
	uint32_t stx_dio_offset_align; /* File offset alignment for direct I/O */

	uint64_t reserved2[12]; /* Spare space for future expansion */
} struct_statx;

#endif /* !STRACE_STATX_H */
