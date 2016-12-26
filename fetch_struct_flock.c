/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
