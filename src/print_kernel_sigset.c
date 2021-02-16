/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_sigset_addr_size)

typedef struct {
	sigset_t *sigmask;
	size_t sigsetsize;
} struct_sigset_addr_size;

#include MPERS_DEFS

MPERS_PRINTER_DECL(void, print_kernel_sigset, struct tcb *tcp,
		   const kernel_ulong_t addr)
{
	struct_sigset_addr_size sas;

	if (!umove_or_printaddr(tcp, addr, &sas)) {
		tprint_struct_begin();
		tprints_field_name("sigmask");
		print_sigset_addr_len(tcp, (uintptr_t) sas.sigmask,
				      sas.sigsetsize);
		tprint_struct_next();
		PRINT_FIELD_U(sas, sigsetsize);
		tprint_struct_end();
	}
}
