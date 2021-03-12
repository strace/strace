/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
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
		/* pathname */
		printpath(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* statbuf */
		print_old_kernel_stat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(oldfstat)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();
	} else {
		/* statbuf */
		print_old_kernel_stat(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#endif /* HAVE_STRUCT___OLD_KERNEL_STAT */
