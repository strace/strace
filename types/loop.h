/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_TYPES_LOOP_H
# define STRACE_TYPES_LOOP_H

# include <linux/ioctl.h>
# include <linux/loop.h>

typedef struct {
	uint32_t fd;
	uint32_t block_size;
	struct loop_info64 info;
	uint64_t __reserved[8];
} struct_loop_config;

#endif /* STRACE_TYPES_LOOP_H */
