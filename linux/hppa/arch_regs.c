/*
 * Copyright (c) 2015-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

static unsigned long hppa_r28;

#define PT_GR20 offsetof(struct pt_regs, gr[20])
#define PT_GR28 offsetof(struct pt_regs, gr[28])

#define ARCH_PC_PEEK_ADDR offsetof(struct pt_regs, iaoq[0])
#define ARCH_SP_PEEK_ADDR offsetof(struct pt_regs, gr[30])
