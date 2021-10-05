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

void qualify_secontext(const char *const str);

enum secontext_bits {
	/* Display full context instead of type only */
	SECONTEXT_FULL,
	/* Check for context mismatch */
	SECONTEXT_MISMATCH,

	NUMBER_OF_SECONTEXT_BITS
};

extern struct number_set *secontext_set;

int selinux_getfdcon(pid_t pid, int fd, char **context);
int selinux_getfilecon(struct tcb *tcp, const char *path, char **context);
int selinux_getpidcon(struct tcb *tcp, char **context);
void selinux_set_format(const char *optarg);

#endif /* !STRACE_SECONTEXT_H */
