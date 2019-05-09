/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#if HAVE_ARCH_TIME32_SYSCALLS

# define TIMESPEC_T kernel_timespec32_t
# define PRINT_TIMESPEC_DATA_SIZE print_timespec32_data_size
# define PRINT_TIMESPEC_ARRAY_DATA_SIZE print_timespec32_array_data_size
# define PRINT_TIMESPEC print_timespec32
# define SPRINT_TIMESPEC sprint_timespec32
# define PRINT_TIMESPEC_UTIME_PAIR print_timespec32_utime_pair
# define PRINT_ITIMERSPEC print_itimerspec32

# include "kernel_timespec.h"
# include "print_timespec.h"

#endif /* HAVE_ARCH_TIME32_SYSCALLS */
