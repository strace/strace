/*
 * Copyright (c) 2021 The strace developers.
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ptrace.h"
#include "arch_prstatus_regset.h"
#include "arch_fpregset.h"

#if HAVE_ARCH_PRSTATUS_REGSET
# include DEF_MPERS_TYPE(struct_prstatus_regset)
#endif

#if HAVE_ARCH_FPREGSET
# include DEF_MPERS_TYPE(struct_fpregset)
#endif

#include MPERS_DEFS

#include "arch_prstatus_regset.c"
#include "arch_fpregset.c"
#include "arch_pt_regs.c"
#include "arch_pt_fpregs.c"

MPERS_PRINTER_DECL(void, decode_prstatus_regset,
		   struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const kernel_ulong_t size)
{
	arch_decode_prstatus_regset(tcp, addr, size);
}

MPERS_PRINTER_DECL(void, decode_fpregset,
		   struct tcb *const tcp,
		   const kernel_ulong_t addr,
		   const kernel_ulong_t size)
{
	arch_decode_fpregset(tcp, addr, size);
}

MPERS_PRINTER_DECL(void, decode_pt_regs,
		   struct tcb *const tcp,
		   const kernel_ulong_t addr)
{
	arch_decode_pt_regs(tcp, addr);
}

MPERS_PRINTER_DECL(void, decode_pt_fpregs,
		   struct tcb *const tcp,
		   const kernel_ulong_t addr)
{
	arch_decode_pt_fpregs(tcp, addr);
}
