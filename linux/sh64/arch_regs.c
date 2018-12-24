/*
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static unsigned long sh64_r9;
#define ARCH_PC_PEEK_ADDR REG_PC
#define ARCH_SP_PEEK_ADDR REG_GENERAL(15)
