/*
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static struct user_regs_struct hppa_regs;
#define ARCH_REGS_FOR_GETREGSET hppa_regs
#define ARCH_PC_REG hppa_regs.iaoq[0]
#define ARCH_SP_REG hppa_regs.gr[30]
