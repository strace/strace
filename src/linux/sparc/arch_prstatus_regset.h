/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_ARCH_PRSTATUS_REGSET_H
# define STRACE_ARCH_PRSTATUS_REGSET_H

typedef struct {
	unsigned int g[8];
	unsigned int o[8];
	unsigned int l[8];
	unsigned int i[8];
	unsigned int psr;
	unsigned int pc;
	unsigned int npc;
	unsigned int y;
	unsigned int wim;
	unsigned int tbr;
} struct_prstatus_regset;

# define HAVE_ARCH_PRSTATUS_REGSET 1

#endif /* !STRACE_ARCH_PRSTATUS_REGSET_H */
