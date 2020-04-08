/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_regs avr32_regs;
#define ARCH_REGS_FOR_GETREGS avr32_regs
#define ARCH_PC_REG avr32_regs.pc
#define ARCH_SP_REG avr32_regs.sp
