/*
 * Copyright (c) 2018 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_GET_PERSONALITY_H
# define STRACE_GET_PERSONALITY_H

# include "ptrace.h"

extern int
get_personality_from_syscall_info(const struct_ptrace_syscall_info *);

#endif /* !STRACE_GET_PERSONALITY_H */
