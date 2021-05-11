/*
 * Copyright (c) 2021 The strace developers.
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "arch_prstatus_regset.h"

#if HAVE_ARCH_PRSTATUS_REGSET

# include DEF_MPERS_TYPE(struct_prstatus_regset)

#endif

#include MPERS_DEFS

#include "arch_prstatus_regset.c"

MPERS_PRINTER_DECL(void, decode_prstatus_regset,
                   struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const kernel_ulong_t size)
{
	arch_decode_prstatus_regset(tcp, addr, size);
}
