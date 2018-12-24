/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * On i386, pt_regs and user_regs_struct are the same,
 * but on 64 bit x86, user_regs_struct has six more fields:
 * fs_base, gs_base, ds, es, fs, gs.
 * PTRACE_GETREGS fills them too, so struct pt_regs would overflow.
 */
struct i386_user_regs_struct {
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t esi;
	uint32_t edi;
	uint32_t ebp;
	uint32_t eax;
	uint32_t xds;
	uint32_t xes;
	uint32_t xfs;
	uint32_t xgs;
	uint32_t orig_eax;
	uint32_t eip;
	uint32_t xcs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t xss;
};
static union {
	struct user_regs_struct      x86_64_r;
	struct i386_user_regs_struct i386_r;
} x86_regs_union;
#define x86_64_regs x86_regs_union.x86_64_r
#define i386_regs   x86_regs_union.i386_r

static struct iovec x86_io = {
	.iov_base = &x86_regs_union
};

#define ARCH_REGS_FOR_GETREGSET x86_regs_union
#define ARCH_IOVEC_FOR_GETREGSET x86_io
#define ARCH_PC_REG \
	(x86_io.iov_len == sizeof(i386_regs) ? i386_regs.eip : x86_64_regs.rip)
#define ARCH_SP_REG \
	(x86_io.iov_len == sizeof(i386_regs) ? i386_regs.esp : x86_64_regs.rsp)

#undef ARCH_MIGHT_USE_SET_REGS
#define ARCH_MIGHT_USE_SET_REGS 0
