/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_pt_regs xtensa_regs;
#define ARCH_REGS_FOR_GETREGS xtensa_regs
#define ARCH_PC_REG xtensa_regs.pc
#define ARCH_SP_REG xtensa_regs.a[1]
