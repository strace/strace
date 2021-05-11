/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../i386/arch_prstatus_regset.h"
#elif !defined STRACE_ARCH_PRSTATUS_REGSET_H
# define STRACE_ARCH_PRSTATUS_REGSET_H

typedef struct {
	kernel_ulong_t r15;
	kernel_ulong_t r14;
	kernel_ulong_t r13;
	kernel_ulong_t r12;
	kernel_ulong_t rbp;
	kernel_ulong_t rbx;
	kernel_ulong_t r11;
	kernel_ulong_t r10;
	kernel_ulong_t r9;
	kernel_ulong_t r8;
	kernel_ulong_t rax;
	kernel_ulong_t rcx;
	kernel_ulong_t rdx;
	kernel_ulong_t rsi;
	kernel_ulong_t rdi;
	kernel_ulong_t orig_rax;
	kernel_ulong_t rip;
	kernel_ulong_t cs;
	kernel_ulong_t eflags;
	kernel_ulong_t rsp;
	kernel_ulong_t ss;
	kernel_ulong_t fs_base;
	kernel_ulong_t gs_base;
	kernel_ulong_t ds;
	kernel_ulong_t es;
	kernel_ulong_t fs;
	kernel_ulong_t gs;
} struct_prstatus_regset;

# define HAVE_ARCH_PRSTATUS_REGSET 1

#endif /* !STRACE_ARCH_PRSTATUS_REGSET_H */
