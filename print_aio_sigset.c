/*
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(strace_aio_sigset)

typedef struct {
	sigset_t *sigmask;
	size_t sigsetsize;
} strace_aio_sigset;

#include MPERS_DEFS

#include "print_fields.h"

MPERS_PRINTER_DECL(void, print_aio_sigset, struct tcb *tcp,
		   const kernel_ulong_t addr)
{
	strace_aio_sigset sigset;

	if (!umove_or_printaddr(tcp, addr, &sigset)) {
		tprints("{sigmask=");
		print_sigset_addr_len(tcp, (uintptr_t) sigset.sigmask,
				      sigset.sigsetsize);
		PRINT_FIELD_U(", ", sigset, sigsetsize);
		tprints("}");
	}
}
