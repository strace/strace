/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ARCH_FPREGSET_H
# define STRACE_ARCH_FPREGSET_H

typedef struct {
	unsigned int cwd;
	unsigned int swd;
	unsigned int twd;
	unsigned int fip;
	unsigned int fcs;
	unsigned int foo;
	unsigned int fos;
	unsigned int st_space[20];
} struct_fpregset;

# define HAVE_ARCH_FPREGSET 1

#endif /* !STRACE_ARCH_FPREGSET_H */
