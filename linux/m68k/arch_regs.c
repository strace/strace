/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_regs_struct m68k_regs;
#define ARCH_REGS_FOR_GETREGS m68k_regs
#define ARCH_PC_REG m68k_regs.pc
#define ARCH_SP_REG m68k_regs.usp
