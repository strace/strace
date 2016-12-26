/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
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
#include "asm_stat.h"
#include "stat.h"

#ifdef HAVE_STRUCT___OLD_KERNEL_STAT

static void
print_old_kernel_stat(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct __old_kernel_stat buf;
	if (umove_or_printaddr(tcp, addr, &buf))
		return;

	struct strace_stat st = {
		.dev = zero_extend_signed_to_ull(buf.st_dev),
		.ino = zero_extend_signed_to_ull(buf.st_ino),
		.rdev = zero_extend_signed_to_ull(buf.st_rdev),
		.size = zero_extend_signed_to_ull(buf.st_size),
		.mode = zero_extend_signed_to_ull(buf.st_mode),
		.nlink = zero_extend_signed_to_ull(buf.st_nlink),
		.uid = zero_extend_signed_to_ull(buf.st_uid),
		.gid = zero_extend_signed_to_ull(buf.st_gid),
		.atime = sign_extend_unsigned_to_ll(buf.st_atime),
		.ctime = sign_extend_unsigned_to_ll(buf.st_ctime),
		.mtime = sign_extend_unsigned_to_ll(buf.st_mtime)
	};

	print_struct_stat(tcp, &st);
}

SYS_FUNC(oldstat)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_old_kernel_stat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(oldfstat)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_old_kernel_stat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#endif /* HAVE_STRUCT___OLD_KERNEL_STAT */
