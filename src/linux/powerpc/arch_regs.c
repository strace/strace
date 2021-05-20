/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_regs ppc_regs;

#define ARCH_REGS_FOR_GETREGS ppc_regs
#define ARCH_PC_REG ppc_regs.nip
#define ARCH_SP_REG ppc_regs.gpr[1]

#define PPC_TRAP_IS_SCV(trap)	(((trap) & 0xfff0) == 0x3000)
