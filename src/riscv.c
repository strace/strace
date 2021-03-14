/*
 * RISC-V-specific syscall decoders.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef RISCV64

# include "xlat/riscv_flush_icache_flags.h"

SYS_FUNC(riscv_flush_icache)
{
	/* uintptr_t start */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* uintptr_t end */
	printaddr(tcp->u_arg[1]);
	tprint_arg_next();

	/* uintptr_t flags */
	printflags64(riscv_flush_icache_flags, tcp->u_arg[2],
		     "SYS_RISCV_FLUSH_ICACHE_???");

	return RVAL_DECODED;
}

#endif /* RISCV64 */
