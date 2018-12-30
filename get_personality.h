/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_GET_PERSONALITY_H
# define STRACE_GET_PERSONALITY_H

# include "ptrace.h"

extern int
get_personality_from_syscall_info(const struct ptrace_syscall_info *);

#endif /* !STRACE_GET_PERSONALITY_H */
