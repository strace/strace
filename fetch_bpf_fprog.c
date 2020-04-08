/*
 * Copyright (c) 2015-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_sock_fprog)

#include <linux/filter.h>
typedef struct sock_fprog struct_sock_fprog;

#include MPERS_DEFS
#include "bpf_fprog.h"

MPERS_PRINTER_DECL(unsigned int, get_sock_fprog_size, void)
{
	return sizeof(struct_sock_fprog);
}

MPERS_PRINTER_DECL(bool, fetch_bpf_fprog, struct tcb *const tcp,
		   const kernel_ulong_t addr, void *const p)
{
	struct bpf_fprog *pfp = p;
	struct_sock_fprog mfp;

	if ((sizeof(*pfp) == sizeof(mfp))
	    && (offsetof(struct bpf_fprog, filter) ==
		offsetof(struct_sock_fprog, filter)))
		return !umove_or_printaddr(tcp, addr, pfp);

	if (umove_or_printaddr(tcp, addr, &mfp))
		return false;

	pfp->len = mfp.len;
	pfp->filter =
#ifndef IN_MPERS
		(uintptr_t)
#endif
		mfp.filter;
	return true;
}
