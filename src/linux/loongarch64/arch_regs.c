/*
 * Copyright (c) 2021-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_pt_regs loongarch_regs;
#define ARCH_REGS_FOR_GETREGSET loongarch_regs
#define ARCH_PC_REG loongarch_regs.csr_era
#define ARCH_SP_REG loongarch_regs.regs[3]
