/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_regs_struct arc_regs;
#define ARCH_REGS_FOR_GETREGSET arc_regs
#define ARCH_PC_REG arc_regs.efa
#define ARCH_SP_REG arc_regs.scratch.sp
