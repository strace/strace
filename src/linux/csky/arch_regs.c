/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_regs csky_regs;

#define ARCH_REGS_FOR_GETREGSET	csky_regs
#define ARCH_PC_REG		csky_regs.pc
#define ARCH_SP_REG		csky_regs.usp
