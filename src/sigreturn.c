/*
 * Copyright (c) 2015-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2021-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "ptrace.h"
#include "nsig.h"
#include "regs.h"

#if defined HAVE_ASM_SIGCONTEXT_H && !defined HAVE_STRUCT_SIGCONTEXT
# include <asm/sigcontext.h>
#endif

/* The following function might be unused, hence the inline qualifier.  */
static inline void
print_sigmask_addr_size(const void *const addr, const unsigned int size)
{
	tprint_struct_begin();
	tprints_field_name("mask");
	tprints_string(sprintsigmask_n("", addr, size));
	tprint_struct_end();
}

#define tprintsigmask_addr(mask_) \
	print_sigmask_addr_size((mask_), sizeof(mask_))

#include "arch_sigreturn.c"

SYS_FUNC(sigreturn)
{
	arch_sigreturn(tcp);

	return RVAL_DECODED;
}
