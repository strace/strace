/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct pt_all_user_regs ia64_regs;

#define ARCH_REGS_FOR_GETREGS ia64_regs
#define ARCH_PC_REG ia64_regs.br[0]
#define ARCH_SP_REG ia64_regs.gr[12]
