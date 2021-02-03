/*
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1995-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_SYSCALL_H
# define STRACE_LINUX_SYSCALL_H

# include "dummy.h"
# include "sys_func.h"
# include "sen.h"

# if HAVE_ARCH_UID16_SYSCALLS
extern SYS_FUNC(chown16);
extern SYS_FUNC(fchown16);
extern SYS_FUNC(getgroups16);
extern SYS_FUNC(getresuid16);
extern SYS_FUNC(getuid16);
extern SYS_FUNC(setfsuid16);
extern SYS_FUNC(setgroups16);
extern SYS_FUNC(setresuid16);
extern SYS_FUNC(setreuid16);
extern SYS_FUNC(setuid16);
# endif /* HAVE_ARCH_UID16_SYSCALLS */

#endif /* !STRACE_LINUX_SYSCALL_H */
