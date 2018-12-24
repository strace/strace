/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static unsigned long sh_r0;
#define ARCH_PC_PEEK_ADDR (4 * REG_PC)
#define ARCH_SP_PEEK_ADDR (4 * 15)
