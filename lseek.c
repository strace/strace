/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2002-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "xlat/whence_codes.h"

/* Linux kernel has exactly one version of lseek:
 * fs/read_write.c::SYSCALL_DEFINE3(lseek, unsigned, fd, off_t, offset, unsigned, origin)
 * In kernel, off_t is always the same as (kernel's) long
 * (see include/uapi/asm-generic/posix_types.h).
 * Use test/x32_lseek.c to test lseek decoding.
 */
SYS_FUNC(lseek)
{
	printfd(tcp, tcp->u_arg[0]);

	kernel_long_t offset;

#ifndef current_klongsize
	if (current_klongsize < sizeof(kernel_long_t)) {
		offset = (int) tcp->u_arg[1];
	} else
#endif /* !current_klongsize */
	{
		offset = tcp->u_arg[1];
	}

	tprintf(", %" PRI_kld ", ", offset);

	printxval(whence_codes, tcp->u_arg[2], "SEEK_???");

	return RVAL_DECODED;
}

/* llseek syscall takes explicitly two ulong arguments hi, lo,
 * rather than one 64-bit argument for which ULONG_LONG works
 * appropriate for the native byte order.
 *
 * See kernel's fs/read_write.c::SYSCALL_DEFINE5(llseek, ...)
 *
 * hi,lo are "unsigned longs" and combined exactly this way in kernel:
 * ((loff_t) hi << 32) | lo
 * Note that for architectures with kernel's long wider than userspace long
 * (such as x32), combining code will use *kernel's*, i.e. *wide* longs
 * for hi and lo.
 */
SYS_FUNC(llseek)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %lld, ",
			  ((long long) tcp->u_arg[1] << 32)
			| ((long long) tcp->u_arg[2]));
	} else {
		printnum_int64(tcp, tcp->u_arg[3], "%" PRIu64);
		tprints(", ");
		printxval(whence_codes, tcp->u_arg[4], "SEEK_???");
	}
	return 0;
}
