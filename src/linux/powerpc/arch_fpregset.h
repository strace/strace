/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ARCH_FPREGSET_H
# define STRACE_ARCH_FPREGSET_H

typedef struct {
	uint64_t fpr[32];
	uint64_t fpscr;
} struct_fpregset;

# define HAVE_ARCH_FPREGSET 1

#endif /* !STRACE_ARCH_FPREGSET_H */
