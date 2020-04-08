/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_pt_regs nios2_regs;
#define ARCH_REGS_FOR_GETREGSET nios2_regs
#define ARCH_PC_REG nios2_regs.regs[PTR_EA]
#define ARCH_SP_REG nios2_regs.regs[PTR_SP]
