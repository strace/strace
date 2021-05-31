/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ARCH_PT_REGS64_H
# define STRACE_ARCH_PT_REGS64_H

typedef struct {
	unsigned long long gpr[32];
	unsigned long long nip;
	unsigned long long msr;
	unsigned long long orig_gpr3;
	unsigned long long ctr;
	unsigned long long link;
	unsigned long long xer;
	unsigned long long ccr;
	unsigned long long softe;
	unsigned long long trap;
	unsigned long long dar;
	unsigned long long dsisr;
	unsigned long long result;
} struct_pt_regs64;

#endif /* !STRACE_ARCH_PT_REGS64_H */
