/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ASM_STAT_H
# define STRACE_ASM_STAT_H

# undef dev_t
# undef gid_t
# undef ino_t
# undef loff_t
# undef mode_t
# undef nlink_t
# undef off64_t
# undef off_t
# undef time_t
# undef uid_t

# define dev_t		__kernel_dev_t
# define gid_t		__kernel_gid_t
# define ino_t		__kernel_ino_t
# define loff_t		__kernel_loff_t
# define mode_t		__kernel_mode_t
# define nlink_t	__kernel_nlink_t
# define off64_t	__kernel_off64_t
# define off_t		__kernel_off_t
# define time_t		__kernel_time_t
# define uid_t		__kernel_uid_t

# include <asm/stat.h>

# undef dev_t
# undef gid_t
# undef ino_t
# undef loff_t
# undef mode_t
# undef nlink_t
# undef off64_t
# undef off_t
# undef time_t
# undef uid_t

# define dev_t		dev_t
# define gid_t		gid_t
# define ino_t		ino_t
# define loff_t		loff_t
# define mode_t		mode_t
# define nlink_t	nlink_t
# define off64_t	off64_t
# define off_t		off_t
# define time_t		time_t
# define uid_t		uid_t

#endif /* !STRACE_ASM_STAT_H */
