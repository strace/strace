/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_regs sh64_regs;
#define ARCH_REGS_FOR_GETREGS sh64_regs
#define ARCH_PC_REG sh64_regs.pc
#define ARCH_SP_REG sh64_regs.regs[15]
