/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_flock)
#include DEF_MPERS_TYPE(struct_flock64)

#include "flock.h"
typedef struct_kernel_flock struct_flock;
typedef struct_kernel_flock64 struct_flock64;

#include MPERS_DEFS

#define SIZEOF_MEMBER(type, member) \
	sizeof(((type *) NULL)->member)

#define FLOCK_MEMBERS_EQ(type, member) \
	(SIZEOF_MEMBER(struct_kernel_flock64, member) == SIZEOF_MEMBER(type, member) \
	 && offsetof(struct_kernel_flock64, member) == offsetof(type, member))

#define FLOCK_STRUCTS_EQ(type) \
	(sizeof(struct_kernel_flock64) == sizeof(type) \
	 && FLOCK_MEMBERS_EQ(type, l_type) \
	 && FLOCK_MEMBERS_EQ(type, l_whence) \
	 && FLOCK_MEMBERS_EQ(type, l_start) \
	 && FLOCK_MEMBERS_EQ(type, l_len) \
	 && FLOCK_MEMBERS_EQ(type, l_pid))

MPERS_PRINTER_DECL(bool, fetch_struct_flock, struct tcb *const tcp,
		   const kernel_ulong_t addr, void *const p)
{
	struct_kernel_flock64 *pfl = p;
	struct_flock mfl;

	if (FLOCK_STRUCTS_EQ(struct_flock))
		return !umove_or_printaddr(tcp, addr, pfl);

	if (umove_or_printaddr(tcp, addr, &mfl))
		return false;

	pfl->l_type = mfl.l_type;
	pfl->l_whence = mfl.l_whence;
	pfl->l_start = mfl.l_start;
	pfl->l_len = mfl.l_len;
	pfl->l_pid = mfl.l_pid;
	return true;
}

MPERS_PRINTER_DECL(bool, fetch_struct_flock64, struct tcb *const tcp,
		   const kernel_ulong_t addr, void *const p)
{
	struct_kernel_flock64 *pfl = p;
	struct_flock64 mfl;

	if (FLOCK_STRUCTS_EQ(struct_flock64))
		return !umove_or_printaddr(tcp, addr, pfl);

	if (umove_or_printaddr(tcp, addr, &mfl))
		return false;

	pfl->l_type = mfl.l_type;
	pfl->l_whence = mfl.l_whence;
	pfl->l_start = mfl.l_start;
	pfl->l_len = mfl.l_len;
	pfl->l_pid = mfl.l_pid;
	return true;
}
