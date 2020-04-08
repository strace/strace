/*
 * Copyright (c) 2019 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_KERNEL_OLD_TIMEX_H
# define STRACE_KERNEL_OLD_TIMEX_H

# define HAVE_ARCH_TIME32_SYSCALLS 1
# include "kernel_timex.h"
# undef HAVE_ARCH_TIME32_SYSCALLS

typedef
# if SIZEOF_KERNEL_LONG_T == 4 || defined LINUX_MIPSN32
kernel_timex32_t
# elif defined __sparc__
kernel_sparc64_timex_t
# else
kernel_timex64_t
# endif
kernel_old_timex_t;

#endif /* !STRACE_KERNEL_OLD_TIMEX_H */
