/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_DIRENT_H
# define STRACE_KERNEL_DIRENT_H

# include "kernel_types.h"

typedef struct {
	kernel_ulong_t	d_ino;
	kernel_ulong_t	d_off;
	unsigned short	d_reclen;
	char		d_name[1];
} kernel_dirent_t;

typedef struct {
	uint64_t	d_ino;
	uint64_t	d_off;
	unsigned short	d_reclen;
	unsigned char	d_type;
	char		d_name[1];
} kernel_dirent64_t;

#endif /* !STRACE_KERNEL_DIRENT_H */
