/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef HAVE_S390_COMPAT_REGS
typedef struct {
	uint32_t mask;
	uint32_t addr;
} ATTRIBUTE_ALIGNED(8) psw_compat_t;

typedef struct {
	psw_compat_t psw;
	uint32_t gprs[NUM_GPRS];
	uint32_t acrs[NUM_ACRS];
	uint32_t orig_gpr2;
} s390_compat_regs;
#endif

static union {
	s390_compat_regs s390_regs;
	s390_regs s390x_regs;
} s390x_regs_union;

#define s390_regset	s390x_regs_union.s390_regs
#define s390x_regset	s390x_regs_union.s390x_regs

static struct iovec s390x_io = {
	.iov_base = &s390x_regs_union,
};


#define ARCH_REGS_FOR_GETREGSET  s390x_regs_union
#define ARCH_IOVEC_FOR_GETREGSET s390x_io
#define ARCH_PC_REG \
	(s390x_io.iov_len == sizeof(s390_regset) ? \
			     s390_regset.psw.addr : s390x_regset.psw.addr)
#define ARCH_SP_REG \
	(s390x_io.iov_len == sizeof(s390_regset) ? \
			     s390_regset.gprs[15] : s390x_regset.gprs[15])

#define ARCH_PERSONALITY_0_IOV_SIZE sizeof(s390x_regset)
#define ARCH_PERSONALITY_1_IOV_SIZE sizeof(s390_regset)
