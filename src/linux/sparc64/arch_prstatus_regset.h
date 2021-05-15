/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../sparc/arch_prstatus_regset.h"
#elif !defined STRACE_ARCH_PRSTATUS_REGSET_H
# define STRACE_ARCH_PRSTATUS_REGSET_H

typedef struct {
	unsigned long g[8];
	unsigned long o[8];
	unsigned long l[8];
	unsigned long i[8];
	unsigned long tstate;
	unsigned long tpc;
	unsigned long tnpc;
	unsigned long y;
} struct_prstatus_regset;

# define HAVE_ARCH_PRSTATUS_REGSET 1

#endif /* !STRACE_ARCH_PRSTATUS_REGSET_H */
