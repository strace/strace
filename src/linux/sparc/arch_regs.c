/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_regs sparc_regs;

/* Indexes into the pt_regs.u_reg[] array -- UREG_XX from kernel are all off
 * by 1 and use Ix instead of Ox.  These work for both 32 and 64 bit Linux. */
#define U_REG_G1 0
#define U_REG_O0 7
#define U_REG_O1 8
#define U_REG_FP 13

#define ARCH_REGS_FOR_GETREGS sparc_regs
#define ARCH_PC_REG sparc_regs.pc
#define ARCH_SP_REG sparc_regs.u_regs[U_REG_FP]
