/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_X86_64_ASM_STAT_H
# define STRACE_X86_64_ASM_STAT_H

# if defined __x86_64__ && defined __ILP32__
#  define stat redirect_kernel_stat
# endif

# include "linux/asm_stat.h"

# if defined __x86_64__ && defined __ILP32__
#  undef stat
/*
 * This is a replacement for x32 <asm/stat.h> which
 * appears to be wrong in older kernel headers.
 */
struct stat {
	kernel_ulong_t	st_dev;
	kernel_ulong_t	st_ino;
	kernel_ulong_t	st_nlink;
	unsigned int	st_mode;
	unsigned int	st_uid;
	unsigned int	st_gid;
	unsigned int	pad0__;
	kernel_ulong_t	st_rdev;
	kernel_long_t	st_size;
	kernel_long_t	st_blksize;
	kernel_long_t	st_blocks;
	kernel_ulong_t	st_atime;
	kernel_ulong_t	st_atime_nsec;
	kernel_ulong_t	st_mtime;
	kernel_ulong_t	st_mtime_nsec;
	kernel_ulong_t	st_ctime;
	kernel_ulong_t	st_ctime_nsec;
	kernel_long_t	pad1__[3];
};

# endif /* __x86_64__ && __ILP32__ */

#endif /* !STRACE_X86_64_ASM_STAT_H */
