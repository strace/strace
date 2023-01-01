/*
 * Copyright (c) 2023 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_KERNEL_TIME_TYPES_H
# define STRACE_KERNEL_TIME_TYPES_H

# include "kernel_timespec.h"

# if defined HAVE_STRUCT___KERNEL_SOCK_TIMEVAL   \
  || defined HAVE_STRUCT___KERNEL_TIMESPEC
#  include <linux/time_types.h>
# else
#  include <stdint.h>
# endif

# ifndef HAVE_STRUCT___KERNEL_SOCK_TIMEVAL
struct __kernel_sock_timeval {
	int64_t tv_sec;
	int64_t tv_usec;
};
# endif

# ifndef HAVE_STRUCT___KERNEL_TIMESPEC
#  define __kernel_timespec kernel_timespec64_t
# endif

#endif /* !STRACE_KERNEL_TIME_TYPES_H */
