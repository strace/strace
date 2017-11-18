/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2017 The strace developers.
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

#ifdef HAVE_LINUX_MEMFD_H
# include <linux/memfd.h>
#endif

#include "xlat/memfd_create_flags.h"

#ifndef MFD_HUGE_SHIFT
# define MFD_HUGE_SHIFT 26
#endif

#ifndef MFD_HUGE_MASK
# define MFD_HUGE_MASK 0x3f
#endif

SYS_FUNC(memfd_create)
{
	printpathn(tcp, tcp->u_arg[0], 255 - (sizeof("memfd:") - 1));
	tprints(", ");

	unsigned int flags = tcp->u_arg[1];
	const unsigned int mask = MFD_HUGE_MASK << MFD_HUGE_SHIFT;
	const unsigned int hugetlb_value = flags & mask;
	flags &= ~mask;

	if (flags || !hugetlb_value)
		printflags(memfd_create_flags, flags, "MFD_???");

	if (hugetlb_value)
		tprintf("%s%u<<MFD_HUGE_SHIFT",
			flags ? "|" : "",
			hugetlb_value >> MFD_HUGE_SHIFT);

	return RVAL_DECODED | RVAL_FD;
}
