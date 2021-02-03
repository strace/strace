/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

struct arm_pt_regs {
	uint32_t uregs[18];
};
#define ARM_cpsr	uregs[16]
#define ARM_pc		uregs[15]
#define ARM_lr		uregs[14]
#define ARM_sp		uregs[13]
#define ARM_ip		uregs[12]
#define ARM_fp		uregs[11]
#define ARM_r10		uregs[10]
#define ARM_r9		uregs[9]
#define ARM_r8		uregs[8]
#define ARM_r7		uregs[7]
#define ARM_r6		uregs[6]
#define ARM_r5		uregs[5]
#define ARM_r4		uregs[4]
#define ARM_r3		uregs[3]
#define ARM_r2		uregs[2]
#define ARM_r1		uregs[1]
#define ARM_r0		uregs[0]
#define ARM_ORIG_r0	uregs[17]

static union {
	struct user_pt_regs aarch64_r;
	struct arm_pt_regs  arm_r;
} arm_regs_union;
#define aarch64_regs	arm_regs_union.aarch64_r
#define arm_regs	arm_regs_union.arm_r

static struct iovec aarch64_io = {
	.iov_base = &arm_regs_union
};

#define ARCH_REGS_FOR_GETREGSET arm_regs_union
#define ARCH_IOVEC_FOR_GETREGSET aarch64_io
#define ARCH_PC_REG \
	((aarch64_io.iov_len == sizeof(arm_regs)) ? arm_regs.ARM_pc : aarch64_regs.pc)
#define ARCH_SP_REG \
	((aarch64_io.iov_len == sizeof(arm_regs)) ? arm_regs.ARM_sp : aarch64_regs.sp)

#define ARCH_PERSONALITY_0_IOV_SIZE sizeof(aarch64_regs)
#define ARCH_PERSONALITY_1_IOV_SIZE sizeof(arm_regs)
