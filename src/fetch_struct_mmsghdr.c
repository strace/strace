/*
 * Copyright (c) 2016-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_mmsghdr)

#include "msghdr.h"
typedef struct mmsghdr struct_mmsghdr;

#include MPERS_DEFS

/*
 * On success, return the number of fetched bytes.
 * On error, return 0;
 *
 * This function cannot use umove_or_printaddr because
 * it is called by dumpio and therefore cannot print anything.
 */

MPERS_PRINTER_DECL(int, fetch_struct_mmsghdr,
		   struct tcb *const tcp, const kernel_ulong_t addr,
		   void *const p)
{
	struct mmsghdr *p_native = p;
	struct_mmsghdr v_compat;

	if (sizeof(*p_native) == sizeof(v_compat))
		return umove(tcp, addr, p_native) ? 0 : sizeof(*p_native);

	if (umove(tcp, addr, &v_compat))
		return 0;

	p_native->msg_hdr.msg_name = (void *) (unsigned long)
	 v_compat.msg_hdr.msg_name;

	p_native->msg_hdr.msg_namelen =
	 v_compat.msg_hdr.msg_namelen;

	p_native->msg_hdr.msg_iov = (void *) (unsigned long)
	 v_compat.msg_hdr.msg_iov;

	p_native->msg_hdr.msg_iovlen =
	 v_compat.msg_hdr.msg_iovlen;

	p_native->msg_hdr.msg_control = (void *) (unsigned long)
	 v_compat.msg_hdr.msg_control;

	p_native->msg_hdr.msg_controllen =
	 v_compat.msg_hdr.msg_controllen;

	p_native->msg_hdr.msg_flags =
	 v_compat.msg_hdr.msg_flags;

	p_native->msg_len =
	 v_compat.msg_len;

	return sizeof(v_compat);
}

MPERS_PRINTER_DECL(unsigned int, sizeof_struct_mmsghdr, void)
{
	return sizeof(struct_mmsghdr);
}
