/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define SYSCALL_INVOKE(file, desc, ptr, size) \
	syscall(SYSCALL_NR, SYSCALL_ARG(file, desc), ptr)
#define PRINT_SYSCALL_HEADER(file, desc, size) \
	printf("%s(" SYSCALL_ARG_FMT ", ", SYSCALL_NAME, SYSCALL_ARG(file, desc))

#define STRUCT_STATFS	struct statfs
#ifdef HAVE_STRUCT_STATFS_F_FRSIZE
# define PRINT_F_FRSIZE
#endif
#ifdef HAVE_STRUCT_STATFS_F_FLAGS
# define PRINT_F_FLAGS
#endif
#if defined HAVE_STRUCT_STATFS_F_FSID_VAL
# define PRINT_F_FSID	f_fsid.val
#elif defined HAVE_STRUCT_STATFS_F_FSID___VAL
# define PRINT_F_FSID	f_fsid.__val
#endif

#include "xstatfsx.c"
