/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ARCH_PRSTATUS_REGSET_H
# define STRACE_ARCH_PRSTATUS_REGSET_H

typedef struct {
# ifdef LINUX_MIPSO32
	unsigned long unused[6];
# endif
	unsigned long regs[32];
	unsigned long lo;
	unsigned long hi;
	unsigned long cp0_epc;
	unsigned long cp0_badvaddr;
	unsigned long cp0_status;
	unsigned long cp0_cause;
} struct_prstatus_regset;

# define HAVE_ARCH_PRSTATUS_REGSET 1

#endif /* !STRACE_ARCH_PRSTATUS_REGSET_H */
