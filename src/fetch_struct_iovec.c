/*
 * Copyright (c) The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(tracee_iovec)

#include <sys/uio.h>
typedef struct iovec tracee_iovec;

#include MPERS_DEFS

#include "iovec.h"

MPERS_PRINTER_DECL(bool, fetch_struct_iovec, struct tcb *const tcp,
		   const kernel_ulong_t addr, void *const dest)
{
	strace_iovec *p = dest;
	tracee_iovec iov;

	if (sizeof(*p) == sizeof(iov))
		return tfetch_mem_ignore_syserror(tcp, addr, sizeof(*p), p);

	if (!tfetch_mem_ignore_syserror(tcp, addr, sizeof(iov), &iov))
		return false;

	p->iov_base = (unsigned long) iov.iov_base;
	p->iov_len = iov.iov_len;

	return true;
}
