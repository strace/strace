/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_regs arm_regs;

#define ARCH_REGS_FOR_GETREGS arm_regs
#define ARCH_PC_REG arm_regs.ARM_pc
#define ARCH_SP_REG arm_regs.ARM_sp
