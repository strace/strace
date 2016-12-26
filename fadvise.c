/*
 * Copyright (c) 2004 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Daniel Jacobowitz  <dan@codesourcery.com>
 * Copyright (c) 2009 Andreas Schwab <schwab@redhat.com>
 * Copyright (c) 2009 Kirill A. Shutemov <kirill@shutemov.name>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#include <fcntl.h>

#include "xlat/advise.h"

SYS_FUNC(fadvise64)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld", 1);
	tprintf(", %" PRI_klu ", ", tcp->u_arg[argn++]);
	printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");

	return RVAL_DECODED;
}

SYS_FUNC(fadvise64_64)
{
	int argn;

	printfd(tcp, tcp->u_arg[0]);
	argn = printllval(tcp, ", %lld, ", 1);
	argn = printllval(tcp, "%lld, ", argn);
#if defined __ARM_EABI__ || defined AARCH64 || defined POWERPC || defined XTENSA
	printxval(advise, tcp->u_arg[1], "POSIX_FADV_???");
#else
	printxval(advise, tcp->u_arg[argn], "POSIX_FADV_???");
#endif

	return RVAL_DECODED;
}
