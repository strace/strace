/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_regs_struct i386_regs;

#define ARCH_REGS_FOR_GETREGS i386_regs
#define ARCH_PC_REG i386_regs.eip
#define ARCH_SP_REG i386_regs.esp

#undef ARCH_MIGHT_USE_SET_REGS
#define ARCH_MIGHT_USE_SET_REGS 0
