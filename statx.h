/*
 * Copyright (c) 2017 The strace developers.
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

#ifndef STRACE_STATX_H
#define STRACE_STATX_H

#include <stdint.h>

typedef struct {
	int64_t sec;
	int32_t nsec;
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

	uint64_t reserved2[14]; /* Spare space for future expansion */
} struct_statx;

#endif /* !STRACE_STATX_H */
