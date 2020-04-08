/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
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

#include "arch_sigreturn.c"

SYS_FUNC(sigreturn)
{
	arch_sigreturn(tcp);

	return RVAL_DECODED;
}
