/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_BTRFS_H
# define STRACE_TYPES_BTRFS_H

# ifdef HAVE_LINUX_BTRFS_H
#  include <stdio.h>
#  include <stdint.h>
#  include <linux/btrfs.h>
# endif

typedef struct {
	uint64_t logical;
	uint64_t size;
	uint64_t reserved[3];
	uint64_t flags;
	uint64_t inodes;
} struct_btrfs_ioctl_logical_ino_args;

#endif /* STRACE_TYPES_BTRFS_H */
