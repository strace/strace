/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_regs_struct e2k_regs;
#define ARCH_REGS_FOR_GETREGS e2k_regs
#define ARCH_PC_REG e2k_regs.cr0_hi
