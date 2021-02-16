/*
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static unsigned long alpha_r0;
static unsigned long alpha_a3;

#define REG_R0 0
#define REG_A0 16
#define REG_A3 19
#define REG_SP 30
#define REG_PC 64

#define ARCH_PC_PEEK_ADDR REG_PC
#define ARCH_SP_PEEK_ADDR REG_SP
