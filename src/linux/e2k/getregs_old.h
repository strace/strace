/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#define HAVE_GETREGS_OLD
static int getregs_old(struct tcb *);
static int set_regs(pid_t);
