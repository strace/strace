/*
 * SELinux interface.
 *
 * Copyright (c) 2020-2021 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SECONTEXT_H
# define STRACE_SECONTEXT_H

# include "defs.h"

extern bool selinux_context;
extern bool selinux_context_full;

int selinux_getfdcon(pid_t pid, int fd, char **context);
int selinux_getfilecon(struct tcb *tcp, const char *path, char **context);
int selinux_getpidcon(struct tcb *tcp, char **context);

#endif /* !STRACE_SECONTEXT_H */
