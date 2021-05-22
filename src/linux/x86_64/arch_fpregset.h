/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../i386/arch_fpregset.h"
#elif !defined STRACE_ARCH_FPREGSET_H
# define STRACE_ARCH_FPREGSET_H

typedef struct {
	uint16_t cwd;
	uint16_t swd;
	uint16_t ftw;
	uint16_t fop;
	uint64_t rip;
	uint64_t rdp;
	uint32_t mxcsr;
	uint32_t mxcr_mask;
	uint32_t st_space[32];
	uint32_t xmm_space[64];
	uint32_t padding[24];
} struct_fpregset;

# define HAVE_ARCH_FPREGSET 1

#endif /* !STRACE_ARCH_FPREGSET_H */
