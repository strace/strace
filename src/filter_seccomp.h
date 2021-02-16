/*
 * Copyright (c) 2018 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SECCOMP_FILTER_H
# define STRACE_SECCOMP_FILTER_H

# include "defs.h"

extern bool seccomp_filtering;
extern bool seccomp_before_sysentry;

extern void check_seccomp_filter(void);
extern void init_seccomp_filter(void);
extern int seccomp_filter_restart_operator(const struct tcb *);

#endif /* !STRACE_SECCOMP_FILTER_H */
