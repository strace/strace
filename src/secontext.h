/*
 * SELinux interface.
 *
 * Copyright (c) 2020-2022 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_SECONTEXT_H
# define STRACE_SECONTEXT_H

# include "defs.h"

void qualify_secontext(const char *const str);

# ifdef ENABLE_SECONTEXT

enum secontext_bits {
	/* Display full context instead of type only */
	SECONTEXT_FULL,
	/* Check for context mismatch */
	SECONTEXT_MISMATCH,

	NUMBER_OF_SECONTEXT_BITS
};

extern struct number_set *secontext_set;

void selinux_printfdcon(pid_t pid, int fd);
void selinux_printfilecon(struct tcb *tcp, const char *path);
void selinux_printpidcon(struct tcb *tcp);

# else

static inline void selinux_printfdcon(pid_t pid, int fd) {}
static inline void selinux_printfilecon(struct tcb *tcp, const char *path) {}
static inline void selinux_printpidcon(struct tcb *tcp) {}

# endif /* ENABLE_SECONTEXT */

#endif /* !STRACE_SECONTEXT_H */
