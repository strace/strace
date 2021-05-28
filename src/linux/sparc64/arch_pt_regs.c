/*
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifdef MPERS_IS_m32
# include "../sparc/arch_pt_regs.c"
#else

static void
arch_decode_pt_regs(struct tcb *const tcp, const kernel_ulong_t addr)
{
	printaddr(addr);
}

#endif /* !MPERS_IS_m32 */
