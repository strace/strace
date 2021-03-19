/*
 * SELinux interface.
 *
 * Copyright (c) 2020-2021 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SELINUX_H
#define STRACE_SELINUX_H

#include "defs.h"

extern bool selinux_context;
extern bool selinux_context_full;

int selinux_getfdcon(pid_t pid, int fd, char **result);
int selinux_getfilecon(struct tcb *tcp, const char *path, char **context);
int selinux_getpidcon(struct tcb *tcp, char **context);

#endif /* !STRACE_SELINUX_H */
