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

#include "xlat/whence_codes.h"

/* Linux kernel has exactly one version of lseek:
 * fs/read_write.c::SYSCALL_DEFINE3(lseek, unsigned, fd, off_t, offset, unsigned, origin)
 * In kernel, off_t is always the same as (kernel's) long
 * (see include/uapi/asm-generic/posix_types.h),
 * which means that on x32 we need to use tcp->ext_arg[N] to get offset argument.
 * Use test/x32_lseek.c to test lseek decoding.
 */
#if HAVE_STRUCT_TCB_EXT_ARG
SYS_FUNC(lseek)
{
	printfd(tcp, tcp->u_arg[0]);

	long long offset;
# if SUPPORTED_PERSONALITIES > 1
	/* tcp->ext_arg is not initialized for compat personality */
	if (current_personality == 1) {
		offset = tcp->u_arg[1];
	} else
# endif
	{
		offset = tcp->ext_arg[1];
	}
	int whence = tcp->u_arg[2];

	tprintf(", %lld, ", offset);
	printxval(whence_codes, whence, "SEEK_???");

	return RVAL_DECODED | RVAL_LUDECIMAL;
}
#else
SYS_FUNC(lseek)
{
	printfd(tcp, tcp->u_arg[0]);

	long offset =
# if SUPPORTED_PERSONALITIES > 1 && SIZEOF_LONG > 4
#  ifdef X86_64
		current_personality == 1 ?
			(long)(int) tcp->u_arg[1] : tcp->u_arg[1];
#  else
		current_wordsize == 4 ?
			(long)(int) tcp->u_arg[1] : tcp->u_arg[1];
#  endif
# else
		tcp->u_arg[1];
# endif
	int whence = tcp->u_arg[2];

	tprintf(", %ld, ", offset);
	printxval(whence_codes, whence, "SEEK_???");

	return RVAL_DECODED | RVAL_UDECIMAL;
}
#endif

/* llseek syscall takes explicitly two ulong arguments hi, lo,
 * rather than one 64-bit argument for which LONG_LONG works
 * appropriate for the native byte order.
 *
 * See kernel's fs/read_write.c::SYSCALL_DEFINE5(llseek, ...)
 *
 * hi,lo are "unsigned longs" and combined exactly this way in kernel:
 * ((loff_t) hi << 32) | lo
 * Note that for architectures with kernel's long wider than userspace long
 * (such as x32), combining code will use *kernel's*, i.e. *wide* longs
 * for hi and lo. We would need to use tcp->ext_arg[N] on x32...
 * ...however, x32 (and x86_64) does not _have_ llseek syscall as such.
 */
SYS_FUNC(llseek)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprintf(", %lld, ",
			(zero_extend_signed_to_ull(tcp->u_arg[1]) << 32)
			| zero_extend_signed_to_ull(tcp->u_arg[2]));
	} else {
		printnum_int64(tcp, tcp->u_arg[3], "%" PRIu64);
		tprints(", ");
		printxval(whence_codes, tcp->u_arg[4], "SEEK_???");
	}
	return 0;
}
