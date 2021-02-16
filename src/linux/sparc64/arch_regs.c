/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "../sparc/arch_regs.c"
#undef ARCH_PC_REG
#define ARCH_PC_REG sparc_regs.tpc
