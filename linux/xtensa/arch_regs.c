/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static unsigned long xtensa_a2;
#define ARCH_PC_PEEK_ADDR REG_PC
#define ARCH_SP_PEEK_ADDR (REG_A_BASE + 1)
